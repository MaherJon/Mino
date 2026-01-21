#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "codegen.h"

// Simple x86_64 assembly backend (AT&T syntax), generates position-dependent executables

typedef struct {
    FILE* out;
    int labelCount;
    // current function parameter and local name maps
    char** paramNames;
    int paramCount;
    char** localNames;
    int localCount;
    // string literal table
    char** strLits;
    int strCount;
} CGContext;

// Flatten dotted names to underscore style for external symbols
static char* flattenName(const char* name) {
    size_t n = strlen(name);
    char* res = malloc(n + 1);
    for (size_t i = 0; i < n; i++) {
        res[i] = (name[i] == '.') ? '_' : name[i];
    }
    res[n] = '\0';
    return res;
}

// String literal table helpers
static int findStringLiteral(CGContext* ctx, const char* s) {
    if (!ctx->strLits) return -1;
    for (int i = 0; i < ctx->strCount; i++) {
        if (strcmp(ctx->strLits[i], s) == 0) return i;
    }
    return -1;
}

static int addStringLiteral(CGContext* ctx, const char* s) {
    int idx = findStringLiteral(ctx, s);
    if (idx >= 0) return idx;
    ctx->strLits = realloc(ctx->strLits, sizeof(char*) * (ctx->strCount + 1));
    ctx->strLits[ctx->strCount] = strdup(s);
    return ctx->strCount++;
}

// Walk AST and collect string literals
static void collectStringLiterals(CGContext* ctx, ASTNode* node) {
    if (!node) return;
    switch (node->type) {
        case NODE_LITERAL: {
            Token t = node->literal.token;
            if (t.type == TOKEN_STRING) {
                // copy token contents
                char* s = malloc(t.length + 1);
                memcpy(s, t.start, t.length);
                s[t.length] = '\0';
                addStringLiteral(ctx, s);
                free(s);
            }
            break;
        }
        case NODE_PROGRAM: {
            for (int i = 0; i < node->program.count; i++) collectStringLiterals(ctx, node->program.statements[i]);
            break;
        }
        case NODE_FUNCTION_DECL: {
            collectStringLiterals(ctx, node->function.returnType);
            for (int i = 0; i < node->function.paramCount; i++) collectStringLiterals(ctx, node->function.params[i]);
            collectStringLiterals(ctx, node->function.body);
            break;
        }
        case NODE_VAR_DECL: {
            collectStringLiterals(ctx, node->variable.type);
            collectStringLiterals(ctx, node->variable.initializer);
            break;
        }
        case NODE_CALL_EXPR: {
            collectStringLiterals(ctx, node->call.callee);
            for (int i = 0; i < node->call.argCount; i++) collectStringLiterals(ctx, node->call.args[i]);
            break;
        }
        case NODE_GET_EXPR: {
            collectStringLiterals(ctx, node->get.object);
            break;
        }
        case NODE_BINARY_EXPR: {
            collectStringLiterals(ctx, node->binary.left);
            collectStringLiterals(ctx, node->binary.right);
            break;
        }
        case NODE_RETURN_STMT: {
            collectStringLiterals(ctx, node->returnStmt.value);
            break;
        }
        default: break;
    }
}

// Get callee name for VARIABLE or GET_EXPR
static char* getCalleeName(ASTNode* callee) {
    if (!callee) return NULL;
    if (callee->type == NODE_VARIABLE) return strdup(callee->varRef.name);
    if (callee->type == NODE_GET_EXPR) {
        // Recursively construct chained name
        char* left = getCalleeName(callee->get.object);
        if (!left) return NULL;
        size_t len = strlen(left) + 1 + strlen(callee->get.name) + 1;
        char* buf = malloc(len);
        snprintf(buf, len, "%s.%s", left, callee->get.name);
        free(left);
        return buf;
    }
    return NULL;
}

// Emit function prologue/epilogue
static void emitPrologue(CGContext* ctx) {
    fprintf(ctx->out, "\tpush %%rbp\n");
    fprintf(ctx->out, "\tmov %%rsp, %%rbp\n");
}

static void emitEpilogue(CGContext* ctx) {
    fprintf(ctx->out, "\tleave\n");
    fprintf(ctx->out, "\tret\n");
}

// Generate expression, result placed in rax
static void genExpression(CGContext* ctx, ASTNode* node, const char* funcName);

// Generate simple stack frame and variable layout for a function, then emit code
static void genFunction(CGContext* ctx, ASTNode* func) {
    // func->function.name, params in func->function.params
    fprintf(ctx->out, "\t.globl %s\n", func->function.name);
    fprintf(ctx->out, "%s:\n", func->function.name);
    emitPrologue(ctx);

    // Collect local variables (var declarations) and record names
    int localCount = 0;
    for (int i = 0; i < func->function.body->program.count; i++) {
        ASTNode* s = func->function.body->program.statements[i];
        if (s && s->type == NODE_VAR_DECL) localCount++;
    }

    // store parameter and local name maps in context for genExpression access
    ctx->paramCount = func->function.paramCount;
    if (ctx->paramNames) { /* clear any previous */
        for (int i = 0; i < ctx->paramCount; i++) if (ctx->paramNames[i]) free(ctx->paramNames[i]);
        free(ctx->paramNames);
        ctx->paramNames = NULL;
    }
    if (func->function.paramCount > 0) {
        ctx->paramNames = malloc(sizeof(char*) * func->function.paramCount);
        for (int i = 0; i < func->function.paramCount; i++) {
            ctx->paramNames[i] = strdup(func->function.params[i]->variable.name);
        }
    }

    ctx->localCount = localCount;
    if (ctx->localNames) {
        for (int i = 0; i < ctx->localCount; i++) if (ctx->localNames[i]) free(ctx->localNames[i]);
        free(ctx->localNames);
        ctx->localNames = NULL;
    }
    if (localCount > 0) {
        ctx->localNames = malloc(sizeof(char*) * localCount);
        int idx = 0;
        for (int i = 0; i < func->function.body->program.count; i++) {
            ASTNode* s = func->function.body->program.statements[i];
            if (s && s->type == NODE_VAR_DECL) {
                ctx->localNames[idx++] = strdup(s->variable.name);
            }
        }
    }

    int totalSlots = func->function.paramCount + localCount;
    int stackSize = ((totalSlots * 8) + 15) & ~15; // 16-byte align
    if (stackSize > 0) fprintf(ctx->out, "\tsub $%d, %%rsp\n", stackSize);

    // Store parameters into stack slots (in order), supports up to 6 parameters
    const char* regs[6] = {"%rdi","%rsi","%rdx","%rcx","%r8","%r9"};
    for (int i = 0; i < func->function.paramCount && i < 6; i++) {
        int offset = 8 * (i + 1);
        fprintf(ctx->out, "\tmov %s, -%d(%%rbp)\n", regs[i], offset);
    }

    // If this is main, call initSystem to initialize runtime
    if (strcmp(func->function.name, "main") == 0) {
        fprintf(ctx->out, "\tcall initSystem\n");
    }

    // Generate code for each statement
    for (int i = 0; i < func->function.body->program.count; i++) {
        ASTNode* s = func->function.body->program.statements[i];
        if (!s) continue;
        switch (s->type) {
            case NODE_VAR_DECL: {
                // find local variable index and store initializer
                int varIndex = -1;
                for (int j = 0; j < ctx->localCount; j++) {
                    if (strcmp(ctx->localNames[j], s->variable.name) == 0) { varIndex = j; break; }
                }
                if (varIndex < 0) varIndex = 0;
                int slotIndex = func->function.paramCount + varIndex;
                int offset = 8 * (slotIndex + 1);

                if (s->variable.initializer) {
                    genExpression(ctx, s->variable.initializer, func->function.name);
                    fprintf(ctx->out, "\tmov %%rax, -%d(%%rbp)\n", offset);
                } else {
                    fprintf(ctx->out, "\tmov $0, -%d(%%rbp)\n", offset);
                }
                break;
            }
            case NODE_RETURN_STMT: {
                if (s->returnStmt.value) {
                    genExpression(ctx, s->returnStmt.value, func->function.name);
                }
                emitEpilogue(ctx);
                break;
            }
            default: {
                if (s->type == NODE_CALL_EXPR || s->type == NODE_BINARY_EXPR || s->type == NODE_VARIABLE) {
                        genExpression(ctx, s, func->function.name);
                    }
                break;
            }
        }
    }

    // Default return 0
    fprintf(ctx->out, "\tmov $0, %%rax\n");
    emitEpilogue(ctx);

    // param/local name maps are stored in ctx and freed when overwritten
}

// Generate call, result in rax
static void genCall(CGContext* ctx, ASTNode* node, const char* funcName) {
    // Set arguments into registers (up to 6)
    int argc = node->call.argCount;
    const char* regs[6] = {"%rdi","%rsi","%rdx","%rcx","%r8","%r9"};
    for (int i = 0; i < argc && i < 6; i++) {
        genExpression(ctx, node->call.args[i], funcName);
        fprintf(ctx->out, "\tmov %%rax, %s\n", regs[i]);
    }

    char* name = getCalleeName(node->call.callee);
    if (!name) {
        fprintf(stderr, "Codegen error: unsupported callee\n");
        return;
    }
    char* flat = flattenName(name);
    free(name);

    // If local function (label), call the name directly; otherwise call flattened name
    fprintf(ctx->out, "\tcall %s\n", flat);
    free(flat);
}

// Generate expression
static void genExpression(CGContext* ctx, ASTNode* node, const char* funcName) {
    if (!node) return;
    switch (node->type) {
        case NODE_LITERAL: {
            Token t = node->literal.token;
            if (t.type == TOKEN_NUMBER) {
                // Direct immediate
                char buf[64] = {0};
                snprintf(buf, sizeof(buf), "%.*s", t.length, t.start);
                fprintf(ctx->out, "\tmov $%s, %%rax\n", buf);
            } else if (t.type == TOKEN_STRING) {
                // find string label
                char* s = malloc(t.length + 1);
                memcpy(s, t.start, t.length);
                s[t.length] = '\0';
                int idx = findStringLiteral(ctx, s);
                free(s);
                if (idx >= 0) {
                    fprintf(ctx->out, "\tmov $ .LC%d, %%rax\n", idx);
                } else {
                    fprintf(ctx->out, "\tmov $0, %%rax\n");
                }
            } else {
                fprintf(ctx->out, "\tmov $0, %%rax\n");
            }
            break;
        }
        case NODE_VARIABLE: {
            // Resolve variable slot among parameters and locals
            int slotIndex = -1;
            // search parameters
            for (int i = 0; i < ctx->paramCount; i++) {
                if (ctx->paramNames && strcmp(ctx->paramNames[i], node->varRef.name) == 0) {
                    slotIndex = i;
                    break;
                }
            }
            // search locals if not found
            if (slotIndex == -1) {
                for (int i = 0; i < ctx->localCount; i++) {
                    if (ctx->localNames && strcmp(ctx->localNames[i], node->varRef.name) == 0) {
                        slotIndex = ctx->paramCount + i;
                        break;
                    }
                }
            }

            if (slotIndex == -1) {
                // fallback: external symbol or uninitialized variable
                fprintf(ctx->out, "\t# variable %s not found in locals/params, default 0\n", node->varRef.name);
                fprintf(ctx->out, "\tmov $0, %%rax\n");
            } else {
                int offset = 8 * (slotIndex + 1);
                fprintf(ctx->out, "\tmov -%d(%%rbp), %%rax\n", offset);
            }
            break;
        }
        case NODE_BINARY_EXPR: {
            // Evaluate left and right
            genExpression(ctx, node->binary.left, funcName);
            fprintf(ctx->out, "\tpush %%rax\n");
            genExpression(ctx, node->binary.right, funcName);
            fprintf(ctx->out, "\tpop %%rbx\n");
            switch (node->binary.op.type) {
                case TOKEN_PLUS:
                    fprintf(ctx->out, "\tadd %%rbx, %%rax\n");
                    break;
                case TOKEN_MINUS:
                    // left - right => rbx - rax -> store in rax
                    fprintf(ctx->out, "\tsub %%rax, %%rbx\n");
                    fprintf(ctx->out, "\tmov %%rbx, %%rax\n");
                    break;
                case TOKEN_STAR:
                    fprintf(ctx->out, "\timul %%rbx, %%rax\n");
                    break;
                case TOKEN_SLASH: {
                    // left / right
                    fprintf(ctx->out, "\tmov %%rax, %%rcx\n");
                    fprintf(ctx->out, "\tmov %%rbx, %%rax\n");
                    fprintf(ctx->out, "\tcqo\n");
                    fprintf(ctx->out, "\tidiv %%rcx\n");
                    break;
                }
                default:
                    fprintf(ctx->out, "\t# unsupported binary op\n");
            }
            break;
        }
        case NODE_CALL_EXPR: {
            genCall(ctx, node, funcName);
            break;
        }
        case NODE_GET_EXPR: {
                // If this is a call to sys.* the CALL handling resolves the name.
                // Do not generate separate code here; return placeholder.
            fprintf(ctx->out, "\t# get expr (placeholder) %s\n", node->get.name);
            fprintf(ctx->out, "\tmov $0, %%rax\n");
            break;
        }
        default:
            fprintf(ctx->out, "\t# expr type %d not implemented\n", node->type);
            fprintf(ctx->out, "\tmov $0, %%rax\n");
    }
}

int codegen_generateExecutable(ASTNode* ast, const char* outPath) {
    if (!ast) return 1;
    CGContext ctx;
    ctx.paramNames = NULL;
    ctx.paramCount = 0;
    ctx.localNames = NULL;
    ctx.localCount = 0;
    char asmPath[512];
    snprintf(asmPath, sizeof(asmPath), "build/out.s");
    ctx.out = fopen(asmPath, "w");
    if (!ctx.out) return 1;

    // Collect and emit string literals in .rodata
    ctx.strLits = NULL;
    ctx.strCount = 0;
    collectStringLiterals(&ctx, ast);
    if (ctx.strCount > 0) {
        fprintf(ctx.out, "\t.section .rodata\n");
        for (int i = 0; i < ctx.strCount; i++) {
            fprintf(ctx.out, ".LC%d:\n", i);
            // emit C-style escaped string
            fprintf(ctx.out, "\t.asciz \"%s\"\n", ctx.strLits[i]);
        }
    }

    fprintf(ctx.out, "\t.text\n");
    fprintf(ctx.out, "\t.global main\n");

    // Generate code for each top-level function
    for (int i = 0; i < ast->program.count; i++) {
        ASTNode* s = ast->program.statements[i];
        if (!s) continue;
        if (s->type == NODE_FUNCTION_DECL) {
            genFunction(&ctx, s);
        }
    }

    fclose(ctx.out);

    // free string table
    if (ctx.strLits) {
        for (int i = 0; i < ctx.strCount; i++) free(ctx.strLits[i]);
        free(ctx.strLits);
    }

        // Invoke gcc to link the executable. If a precompiled runtime object exists, prefer it.
        char cmd[1024];
        const char* libArchive = "lib/minolib/libminosys.a";
        const char* runtimeObj = "lib/minolib/System/System.o";
        FILE* farchive = fopen(libArchive, "r");
        if (farchive) {
            fclose(farchive);
            // link with the static archive
            snprintf(cmd, sizeof(cmd), "gcc -no-pie -o %s %s -Llib/minolib -lminosys -I./include -lm", outPath, "build/out.s");
        } else {
            FILE* fobj = fopen(runtimeObj, "r");
            if (fobj) {
                fclose(fobj);
                snprintf(cmd, sizeof(cmd), "gcc -no-pie -o %s %s %s -I./include -lm", outPath, "build/out.s", runtimeObj);
            } else {
                // fallback: compile and link the C source directly
                snprintf(cmd, sizeof(cmd), "gcc -no-pie -o %s %s %s -I./include -lm", outPath, "build/out.s", "lib/minolib/System/System.c");
            }
        }
        int rc = system(cmd);
    if (rc != 0) {
        fprintf(stderr, "Linking failed (rc=%d)\n", rc);
        return 1;
    }

    return 0;
}
