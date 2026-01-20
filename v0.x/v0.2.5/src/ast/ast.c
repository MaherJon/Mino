#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ast.h>
#include <System.h>

// src/ast/ast.c - AST node creation, printing and freeing
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ast.h>
#include <System.h>

// ==================== Helper utilities ====================

// duplicate string
static char* copyString(const char* source) {
    if (source == NULL) return NULL;
    size_t length = strlen(source);
    char* copy = malloc(length + 1);
    if (copy == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    strcpy(copy, source);
    return copy;
}

// Create AST node
static ASTNode* createNode(NodeType type, int line) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (node == NULL) {
        fprintf(stderr, "Memory allocation failed for AST node\n");
        exit(1);
    }
    
    node->type = type;
    node->line = line;
    return node;
}

// ==================== AST creation functions ====================

// Create program node
ASTNode* createProgramNode(ASTNode** statements, int count) {
    ASTNode* node = createNode(NODE_PROGRAM, 0);
    node->program.statements = statements;
    node->program.count = count;
    return node;
}

// Create function declaration node
ASTNode* createFunctionNode(char* name, ASTNode** params, int paramCount, 
                           ASTNode* returnType, ASTNode* body) {
    ASTNode* node = createNode(NODE_FUNCTION_DECL, 0);
    node->function.name = copyString(name);
    node->function.params = params;
    node->function.paramCount = paramCount;
    node->function.returnType = returnType;
    node->function.body = body;
    return node;
}

// Create variable declaration node
ASTNode* createVarNode(char* name, ASTNode* type, ASTNode* initializer) {
    ASTNode* node = createNode(NODE_VAR_DECL, 0);
    node->variable.name = copyString(name);
    node->variable.type = type;
    node->variable.initializer = initializer;
    return node;
}

// Create literal node
ASTNode* createLiteralNode(Token token) {
    ASTNode* node = createNode(NODE_LITERAL, token.line);
    node->literal.token = token;
    return node;
}

// Create variable reference node
ASTNode* createVarRefNode(char* name) {
    ASTNode* node = createNode(NODE_VARIABLE, 0);
    node->varRef.name = copyString(name);
    return node;
}

// Create binary expression node
ASTNode* createBinaryNode(Token op, ASTNode* left, ASTNode* right) {
    ASTNode* node = createNode(NODE_BINARY_EXPR, op.line);
    node->binary.op = op;
    node->binary.left = left;
    node->binary.right = right;
    return node;
}

// Create assignment node
ASTNode* createAssignmentNode(ASTNode* target, ASTNode* value) {
    ASTNode* node = createNode(NODE_ASSIGN, 0);
    if (target != NULL) {
        node->line = target->line;
    }
    node->assignment.target = target;
    node->assignment.value = value;
    return node;
}

// Create return statement node
ASTNode* createReturnNode(ASTNode* value) {
    ASTNode* node = createNode(NODE_RETURN_STMT, 0);
    if (value != NULL) {
        node->line = value->line;
    }
    node->returnStmt.value = value;
    return node;
}

// Create function call node
ASTNode* createCallNode(ASTNode* callee, ASTNode** args, int argCount) {
    ASTNode* node = createNode(NODE_CALL_EXPR, 0);
    node->call.callee = callee;
    node->call.args = args;
    node->call.argCount = argCount;
    return node;
}

// Create member access node (get)
ASTNode* createGetNode(ASTNode* object, char* name) {
    ASTNode* node = createNode(NODE_GET_EXPR, 0);
    node->get.object = object;
    node->get.name = copyString(name);
    return node;
}

// Create include node
ASTNode* createIncludeNode(char* filename) {
    ASTNode* node = createNode(NODE_INCLUDE, 0);
    node->include.filename = copyString(filename);
    return node;
}

// ==================== AST free functions ====================

// Free AST node
void freeAST(ASTNode* node) {
    if (node == NULL) return;
    
    switch (node->type) {
        case NODE_PROGRAM:
            for (int i = 0; i < node->program.count; i++) {
                freeAST(node->program.statements[i]);
            }
            if (node->program.statements != NULL) {
                free(node->program.statements);
            }
            break;
            
        case NODE_FUNCTION_DECL:
            free(node->function.name);
            for (int i = 0; i < node->function.paramCount; i++) {
                freeAST(node->function.params[i]);
            }
            if (node->function.params != NULL) {
                free(node->function.params);
            }
            freeAST(node->function.returnType);
            freeAST(node->function.body);
            break;
            
        case NODE_VAR_DECL:
            free(node->variable.name);
            freeAST(node->variable.type);
            freeAST(node->variable.initializer);
            break;
            
        case NODE_BINARY_EXPR:
            freeAST(node->binary.left);
            freeAST(node->binary.right);
            break;
        case NODE_CALL_EXPR:
            if (node->call.callee) freeAST(node->call.callee);
            for (int i = 0; i < node->call.argCount; i++) {
                freeAST(node->call.args[i]);
            }
            if (node->call.args) free(node->call.args);
            break;

        case NODE_GET_EXPR:
            if (node->get.object) freeAST(node->get.object);
            if (node->get.name) free(node->get.name);
            break;
            
        case NODE_ASSIGN:
            freeAST(node->assignment.target);
            freeAST(node->assignment.value);
            break;
            
        case NODE_RETURN_STMT:
            freeAST(node->returnStmt.value);
            break;
            
        case NODE_VARIABLE:
            free(node->varRef.name);
            break;
            
        case NODE_LITERAL:
            // Tokens do not require special freeing
            break;
            
        case NODE_INCLUDE:
            free(node->include.filename);
            break;
            
        // Other node types do not require special handling for now
        case NODE_CLASS_DECL:
        case NODE_EXPR_STMT:
        case NODE_IF_STMT:
        case NODE_WHILE_STMT:
        case NODE_BLOCK_STMT:
        case NODE_UNARY_EXPR:
        case NODE_SET_EXPR:
            // Free logic for these node types may be added later
            fprintf(stderr, "Warning: freeAST not implemented for node type %d\n", node->type);
            break;
    }
    
    free(node);
}

// ==================== Debug utilities ====================

// Print AST nodes (debug)
static void printIndent(int depth) {
    for (int i = 0; i < depth; i++) {
        printf("  ");
    }
}

void printAST(ASTNode* node, int depth) {
    if (node == NULL) {
        printIndent(depth);
        printf("NULL\n");
        return;
    }
    
    printIndent(depth);
    printf("[Line %d] ", node->line);
    
    switch (node->type) {
        case NODE_PROGRAM:
            printf("Program (%d statements):\n", node->program.count);
            for (int i = 0; i < node->program.count; i++) {
                printAST(node->program.statements[i], depth + 1);
            }
            break;
            
        case NODE_FUNCTION_DECL:
            printf("Function: %s (params: %d)\n", 
                   node->function.name, node->function.paramCount);
            if (node->function.returnType != NULL) {
                printIndent(depth + 1);
                printf("Return Type:\n");
                printAST(node->function.returnType, depth + 2);
            }
            if (node->function.body != NULL) {
                printIndent(depth + 1);
                printf("Body:\n");
                printAST(node->function.body, depth + 2);
            }
            break;
            
        case NODE_VAR_DECL:
            printf("Variable: %s\n", node->variable.name);
            if (node->variable.type != NULL) {
                printIndent(depth + 1);
                printf("Type:\n");
                printAST(node->variable.type, depth + 2);
            }
            if (node->variable.initializer != NULL) {
                printIndent(depth + 1);
                printf("Initializer:\n");
                printAST(node->variable.initializer, depth + 2);
            }
            break;
            
        case NODE_VARIABLE:
            printf("VariableRef: %s\n", node->varRef.name);
            break;
            
        case NODE_LITERAL: {
            Token token = node->literal.token;
            printf("Literal: ");
            if (token.type == TOKEN_NUMBER) {
                printf("Number '%.*s'\n", token.length, token.start);
            } else if (token.type == TOKEN_STRING) {
                printf("String '%.*s'\n", token.length, token.start);
            } else if (token.type == TOKEN_TRUE) {
                printf("true\n");
            } else if (token.type == TOKEN_FALSE) {
                printf("false\n");
            } else if (token.type == TOKEN_NULL) {
                printf("null\n");
            } else {
                printf("Unknown literal type %d\n", token.type);
            }
            break;
        }
            
        case NODE_BINARY_EXPR:
            printf("BinaryExpr: ");
            switch (node->binary.op.type) {
                case TOKEN_PLUS: printf("+\n"); break;
                case TOKEN_MINUS: printf("-\n"); break;
                case TOKEN_STAR: printf("*\n"); break;
                case TOKEN_SLASH: printf("/\n"); break;
                case TOKEN_EQUAL_EQUAL: printf("==\n"); break;
                case TOKEN_BANG_EQUAL: printf("!=\n"); break;
                case TOKEN_GREATER: printf(">\n"); break;
                case TOKEN_GREATER_EQUAL: printf(">=\n"); break;
                case TOKEN_LESS: printf("<\n"); break;
                case TOKEN_LESS_EQUAL: printf("<=\n"); break;
                default: printf("Unknown operator %d\n", node->binary.op.type);
            }
            printIndent(depth + 1);
            printf("Left:\n");
            printAST(node->binary.left, depth + 2);
            printIndent(depth + 1);
            printf("Right:\n");
            printAST(node->binary.right, depth + 2);
            break;

            case NODE_CALL_EXPR:
                printf("CallExpr:\n");
                printIndent(depth + 1);
                printf("Callee:\n");
                printAST(node->call.callee, depth + 2);
                if (node->call.argCount > 0) {
                    printIndent(depth + 1);
                    printf("Args:\n");
                    for (int i = 0; i < node->call.argCount; i++) {
                        printAST(node->call.args[i], depth + 2);
                    }
                }
                break;

            case NODE_GET_EXPR:
                printf("GetExpr: %s\n", node->get.name ? node->get.name : "");
                printIndent(depth + 1);
                printf("Object:\n");
                printAST(node->get.object, depth + 2);
                break;
            
        case NODE_ASSIGN:
            printf("Assignment:\n");
            printIndent(depth + 1);
            printf("Target:\n");
            printAST(node->assignment.target, depth + 2);
            printIndent(depth + 1);
            printf("Value:\n");
            printAST(node->assignment.value, depth + 2);
            break;
            
        case NODE_RETURN_STMT:
            printf("Return:\n");
            if (node->returnStmt.value != NULL) {
                printAST(node->returnStmt.value, depth + 1);
            } else {
                printIndent(depth + 1);
                printf("void\n");
            }
            break;
            
        case NODE_INCLUDE:
            printf("Include: %s\n", node->include.filename);
            break;
            
        default:
            printf("Unknown node type: %d\n", node->type);
    }
}

// ==================== Test functions ====================

#ifdef AST_TEST
// Test AST creation
int main() {
    printf("=== Testing AST Creation ===\n");
    
    // Create literal node
    Token numToken = {TOKEN_NUMBER, "42", 2, 1};
    ASTNode* numNode = createLiteralNode(numToken);
    
    // Create variable reference node
    ASTNode* varNode = createVarRefNode("x");
    
    // Create binary expression node
    Token plusToken = {TOKEN_PLUS, "+", 1, 1};
    ASTNode* addNode = createBinaryNode(plusToken, numNode, varNode);
    
    // Create assignment node
    ASTNode* assignTarget = createVarRefNode("result");
    ASTNode* assignNode = createAssignmentNode(assignTarget, addNode);
    
    // Create variable declaration node
    Token intToken = {TOKEN_INT, "int", 3, 1};
    ASTNode* typeNode = createLiteralNode(intToken);
    ASTNode* varDeclNode = createVarNode("x", typeNode, NULL);
    
    // Create return node
    ASTNode* returnNode = createReturnNode(assignTarget);
    
    // Create function node
    ASTNode* funcBody = assignNode; // Simplified: function body contains one assignment
    ASTNode* funcNode = createFunctionNode("main", NULL, 0, NULL, funcBody);
    
    // Create program node
    ASTNode* statements[2] = {varDeclNode, funcNode};
    ASTNode* programNode = createProgramNode(statements, 2);
    
    // Print AST
    printf("\n=== AST Structure ===\n");
    printAST(programNode, 0);
    
    // Clean up memory
    freeAST(programNode);
    
    printf("\n=== Test Complete ===\n");
    return 0;
}
#endif