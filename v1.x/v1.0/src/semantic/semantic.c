#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semantic.h>
#include <System.h>

// ============ Symbol table implementation ============

#define TABLE_SIZE 64

// Hash function
static unsigned int hash(const char* str) {
    unsigned int hash = 2166136261u;
    while (*str) {
        hash ^= (unsigned char)(*str);
        hash *= 16777619;
        str++;
    }
    return hash;
}

SymbolTable* createSymbolTable() {
    SymbolTable* table = malloc(sizeof(SymbolTable));
    table->capacity = TABLE_SIZE;
    table->count = 0;
    table->scopeDepth = 0;
    table->buckets = calloc(TABLE_SIZE, sizeof(Symbol*));
    return table;
}

void freeSymbolTable(SymbolTable* table) {
    if (!table) return;
    
    for (int i = 0; i < table->capacity; i++) {
        Symbol* symbol = table->buckets[i];
        while (symbol) {
            Symbol* next = symbol->next;
            free(symbol->name);
            free(symbol);
            symbol = next;
        }
    }
    
    free(table->buckets);
    free(table);
}

int enterScope(SymbolTable* table) {
    if (!table) return 0;
    table->scopeDepth++;
    return 1;
}

int exitScope(SymbolTable* table) {
    if (!table || table->scopeDepth <= 0) return 0;
    
    // Remove all symbols in the current scope
    for (int i = 0; i < table->capacity; i++) {
        Symbol* prev = NULL;
        Symbol* curr = table->buckets[i];
        
        while (curr) {
            if (curr->scopeDepth == table->scopeDepth) {
                // Remove the current symbol
                if (prev) {
                    prev->next = curr->next;
                } else {
                    table->buckets[i] = curr->next;
                }
                
                Symbol* toFree = curr;
                curr = curr->next;
                
                free(toFree->name);
                free(toFree);
                table->count--;
            } else {
                prev = curr;
                curr = curr->next;
            }
        }
    }
    
    table->scopeDepth--;
    return 1;
}

int defineSymbol(SymbolTable* table, const char* name, SymbolType type, 
                 ASTNode* typeNode, int line) {
    if (!table || !name) return 0;
    
    unsigned int index = hash(name) % table->capacity;
    
    // Check if already defined (in the same scope)
    Symbol* existing = table->buckets[index];
    while (existing) {
        if (strcmp(existing->name, name) == 0 && 
            existing->scopeDepth == table->scopeDepth) {
            fprintf(stderr, "[line %d] Error: Symbol '%s' already defined in this scope\n", 
                    line, name);
            return 0;
        }
        existing = existing->next;
    }
    
    // Create new symbol
    Symbol* symbol = malloc(sizeof(Symbol));
    symbol->name = strdup(name);
    symbol->type = type;
    symbol->typeNode = typeNode;
    symbol->scopeDepth = table->scopeDepth;
    symbol->definedLine = line;
    
    // Insert at the head of the bucket list
    symbol->next = table->buckets[index];
    table->buckets[index] = symbol;
    table->count++;
    
    return 1;
}

Symbol* resolveSymbol(SymbolTable* table, const char* name) {
    if (!table || !name) return NULL;
    
    unsigned int index = hash(name) % table->capacity;
    Symbol* symbol = table->buckets[index];
    
    Symbol* found = NULL;
    int foundDepth = -1;
    
    while (symbol) {
        if (strcmp(symbol->name, name) == 0) {
            // Found a symbol in the innermost scope
            if (symbol->scopeDepth > foundDepth) {
                found = symbol;
                foundDepth = symbol->scopeDepth;
            }
        }
        symbol = symbol->next;
    }
    
    return found;
}

// ============ Type checking implementation ============

static TypeInfo* createTypeInfo(const char* name, int size, int isPrimitive) {
    TypeInfo* info = malloc(sizeof(TypeInfo));
    info->name = strdup(name);
    info->size = size;
    info->isArray = 0;
    info->isPrimitive = isPrimitive;
    info->base = NULL;
    return info;
}

static void freeTypeInfo(TypeInfo* info) {
    if (!info) return;
    free(info->name);
    free(info);
}

TypeInfo* getTypeInfo(ASTNode* node, SymbolTable* symbols) {
    if (!node) return NULL;
    
    switch (node->type) {
        case NODE_LITERAL: {
            Token token = node->literal.token;
            switch (token.type) {
                case TOKEN_NUMBER:
                    // Check for decimal point
                    for (int i = 0; i < token.length; i++) {
                        if (token.start[i] == '.') {
                            return createTypeInfo("float", sizeof(float), 1);
                        }
                    }
                    return createTypeInfo("int", sizeof(int), 1);
                case TOKEN_STRING:
                    return createTypeInfo("string", sizeof(char*), 1);
                case TOKEN_TRUE:
                case TOKEN_FALSE:
                    return createTypeInfo("bool", sizeof(int), 1);
                case TOKEN_INT:
                    return createTypeInfo("int", sizeof(int), 1);
                case TOKEN_FLOAT:
                    return createTypeInfo("float", sizeof(float), 1);
                case TOKEN_BOOL:
                    return createTypeInfo("bool", sizeof(int), 1);
                case TOKEN_STRING_TYPE:
                    return createTypeInfo("string", sizeof(char*), 1);
                case TOKEN_VOID:
                    return createTypeInfo("void", 0, 1);
                default:
                    return NULL;
            }
        }
            
        case NODE_VARIABLE: {
            Symbol* symbol = resolveSymbol(symbols, node->varRef.name);
            if (!symbol) return NULL;

            // If this is a function symbol, return the function's return type
            if (symbol->type == SYM_FUNCTION && symbol->typeNode &&
                symbol->typeNode->type == NODE_FUNCTION_DECL) {
                ASTNode* func = symbol->typeNode;
                return getTypeInfo(func->function.returnType, symbols);
            }

            if (symbol->typeNode && symbol->typeNode->type == NODE_LITERAL) {
                Token typeToken = symbol->typeNode->literal.token;
                switch (typeToken.type) {
                    case TOKEN_INT: return createTypeInfo("int", sizeof(int), 1);
                    case TOKEN_FLOAT: return createTypeInfo("float", sizeof(float), 1);
                    case TOKEN_BOOL: return createTypeInfo("bool", sizeof(int), 1);
                    case TOKEN_STRING_TYPE: return createTypeInfo("string", sizeof(char*), 1);
                    case TOKEN_VOID: return createTypeInfo("void", 0, 1);
                    default: return NULL;
                }
            }
            return NULL;
        }
            
        case NODE_BINARY_EXPR: {
            TypeInfo* leftType = getTypeInfo(node->binary.left, symbols);
            TypeInfo* rightType = getTypeInfo(node->binary.right, symbols);
            
            if (!leftType || !rightType) {
                if (leftType) freeTypeInfo(leftType);
                if (rightType) freeTypeInfo(rightType);
                return NULL;
            }
            
            // Check type compatibility
            if (strcmp(leftType->name, rightType->name) != 0) {
                fprintf(stderr, "[line %d] Error: Type mismatch in binary expression\n", 
                        node->line);
                freeTypeInfo(leftType);
                freeTypeInfo(rightType);
                return NULL;
            }
            
            freeTypeInfo(rightType);
            return leftType; // Return left operand type
        }

        case NODE_GET_EXPR: {
            // Resolve symbol by full chained name, e.g. sys.IO.print -> "sys.IO.print"
            // Build the chained name
            char* fullname = NULL;
            // Helper: recursive name construction
            ASTNode* cur = node;
            // Use a small recursive helper to generate fullname
            function_char_concat:;
            ;
            // Recursively obtain object name
            char* getNameRecursive(ASTNode* n) {
                if (!n) return NULL;
                if (n->type == NODE_VARIABLE) {
                    return strdup(n->varRef.name);
                }
                if (n->type == NODE_GET_EXPR) {
                    char* left = getNameRecursive(n->get.object);
                    if (!left) return NULL;
                    int len = strlen(left) + 1 + strlen(n->get.name) + 1;
                    char* res = malloc(len);
                    snprintf(res, len, "%s.%s", left, n->get.name);
                    free(left);
                    return res;
                }
                return NULL;
            }

            fullname = getNameRecursive(node->get.object);
            if (!fullname) return NULL;
            int needed = strlen(fullname) + 1 + strlen(node->get.name) + 1;
            char* total = malloc(needed);
            snprintf(total, needed, "%s.%s", fullname, node->get.name);
            free(fullname);

            Symbol* symbol = resolveSymbol(symbols, total);
            free(total);
            if (!symbol) return NULL;

            // If the symbol is a function, return its return type
            if (symbol->type == SYM_FUNCTION && symbol->typeNode &&
                symbol->typeNode->type == NODE_FUNCTION_DECL) {
                return getTypeInfo(symbol->typeNode->function.returnType, symbols);
            }

            if (symbol->typeNode && symbol->typeNode->type == NODE_LITERAL) {
                return getTypeInfo(symbol->typeNode, symbols);
            }

            return NULL;
        }

        case NODE_CALL_EXPR: {
            // callee should be VARIABLE or a GET_EXPR chain; resolve its symbol
            // First try resolving by callee name
            char* calleeName = NULL;
            char* getNameRecursive2(ASTNode* n) {
                if (!n) return NULL;
                if (n->type == NODE_VARIABLE) return strdup(n->varRef.name);
                if (n->type == NODE_GET_EXPR) {
                    char* left = getNameRecursive2(n->get.object);
                    if (!left) return NULL;
                    int len = strlen(left) + 1 + strlen(n->get.name) + 1;
                    char* res = malloc(len);
                    snprintf(res, len, "%s.%s", left, n->get.name);
                    free(left);
                    return res;
                }
                return NULL;
            }

            calleeName = getNameRecursive2(node->call.callee);
            if (!calleeName) return NULL;

            Symbol* symbol = resolveSymbol(symbols, calleeName);
            // If symbol not found, allow calls to runtime 'sys' namespace as external functions
            if (!symbol) {
                // check if calleeName starts with "sys" (e.g., sys.Math.powInt or sys_IO_print_PrintInt)
                if (strncmp(calleeName, "sys", 3) == 0) {
                    // naive external function: ensure argument types are inferable (prefer int)
                    for (int i = 0; i < node->call.argCount; i++) {
                        TypeInfo* at = getTypeInfo(node->call.args[i], symbols);
                        if (!at) {
                            // cannot determine arg type -> fail
                            free(calleeName);
                            return NULL;
                        }
                        // accept ints and floats for now
                        if (strcmp(at->name, "int") != 0 && strcmp(at->name, "float") != 0) {
                            freeTypeInfo(at);
                            free(calleeName);
                            return NULL;
                        }
                        freeTypeInfo(at);
                    }
                    // Default to int return type for integer-friendly runtime helpers
                    free(calleeName);
                    return createTypeInfo("int", sizeof(int), 1);
                }
                free(calleeName);
                fprintf(stderr, "[line %d] Error: Undefined function in call\n", node->line);
                return NULL;
            }

            if (symbol->type != SYM_FUNCTION || !symbol->typeNode ||
                symbol->typeNode->type != NODE_FUNCTION_DECL) {
                fprintf(stderr, "[line %d] Error: Called symbol is not a function\n", node->line);
                return NULL;
            }

            ASTNode* func = symbol->typeNode;
            int expected = func->function.paramCount;
            if (expected != node->call.argCount) {
                fprintf(stderr, "[line %d] Error: Argument count mismatch in call\n", node->line);
                return NULL;
            }

            // Check parameter types
            for (int i = 0; i < node->call.argCount; i++) {
                TypeInfo* argType = getTypeInfo(node->call.args[i], symbols);
                TypeInfo* paramType = getTypeInfo(func->function.params[i]->variable.type, symbols);
                if (!argType || !paramType) {
                    if (argType) freeTypeInfo(argType);
                    if (paramType) freeTypeInfo(paramType);
                    fprintf(stderr, "[line %d] Error: Cannot determine argument type\n", node->line);
                    return NULL;
                }

                if (!areTypesCompatible(argType, paramType)) {
                    fprintf(stderr, "[line %d] Error: Argument type mismatch\n", node->line);
                    freeTypeInfo(argType);
                    freeTypeInfo(paramType);
                    return NULL;
                }

                freeTypeInfo(argType);
                freeTypeInfo(paramType);
            }

            // Return the function's return type
            return getTypeInfo(func->function.returnType, symbols);
        }
            
        default:
            return NULL;
    }
}

int areTypesCompatible(TypeInfo* t1, TypeInfo* t2) {
    if (!t1 || !t2) return 0;
    return strcmp(t1->name, t2->name) == 0;
}

int typeCheck(ASTNode* node, SymbolTable* symbols) {
    if (!node) return 1;
    
    switch (node->type) {
        case NODE_PROGRAM: {
            // First declare all functions to support forward calls
            for (int i = 0; i < node->program.count; i++) {
                ASTNode* s = node->program.statements[i];
                if (s && s->type == NODE_FUNCTION_DECL) {
                    if (!defineSymbol(symbols, s->function.name, SYM_FUNCTION, s, s->line)) {
                        return 0;
                    }
                }
            }

            // Then perform type checking
            for (int i = 0; i < node->program.count; i++) {
                if (!typeCheck(node->program.statements[i], symbols)) {
                    return 0;
                }
            }
            return 1;
        }
            
        case NODE_VAR_DECL: {
            // If there is an initializer, infer its type first
            if (node->variable.initializer) {
                TypeInfo* initType = getTypeInfo(node->variable.initializer, symbols);
                TypeInfo* declType = getTypeInfo(node->variable.type, symbols);

                if (!initType && !declType) {
                    fprintf(stderr, "[line %d] Error: Cannot determine type (var '%s')\n", node->line, node->variable.name);
                    if (node->variable.type && node->variable.type->type == NODE_LITERAL) {
                        Token t = node->variable.type->literal.token;
                        fprintf(stderr, "  Decl type token: %d '%.*s'\n", t.type, t.length, t.start);
                    } else {
                        fprintf(stderr, "  Decl type node missing or not literal\n");
                    }
                    if (node->variable.initializer) {
                        fprintf(stderr, "  Initializer node type: %d\n", node->variable.initializer->type);
                    }
                    return 0;
                }

                // If no explicit decl type, but can infer from initializer, create a literal type node
                if (!declType && initType) {
                    Token tkn;
                    if (strcmp(initType->name, "int") == 0) tkn.type = TOKEN_INT;
                    else if (strcmp(initType->name, "float") == 0) tkn.type = TOKEN_FLOAT;
                    else if (strcmp(initType->name, "bool") == 0) tkn.type = TOKEN_BOOL;
                    else if (strcmp(initType->name, "string") == 0) tkn.type = TOKEN_STRING_TYPE;
                    else tkn.type = TOKEN_IDENTIFIER;
                    char* s = strdup(initType->name);
                    tkn.start = s;
                    tkn.length = (int)strlen(s);
                    tkn.line = node->line;
                    ASTNode* lit = createLiteralNode(tkn);
                    // createLiteralNode copies token but we need to free allocated start
                    free(s);
                    node->variable.type = lit;
                    // refresh declType
                    freeTypeInfo(initType);
                    initType = NULL;
                }

                // If an explicit type remains, compare compatibility
                if (node->variable.type) {
                    TypeInfo* finalInit = getTypeInfo(node->variable.initializer, symbols);
                    TypeInfo* finalDecl = getTypeInfo(node->variable.type, symbols);
                    if (!finalInit || !finalDecl) {
                        if (finalInit) freeTypeInfo(finalInit);
                        if (finalDecl) freeTypeInfo(finalDecl);
                        fprintf(stderr, "[line %d] Error: Cannot determine type\n", node->line);
                        return 0;
                    }
                    if (!areTypesCompatible(finalInit, finalDecl)) {
                        fprintf(stderr, "[line %d] Error: Type mismatch in variable initialization\n", node->line);
                        freeTypeInfo(finalInit);
                        freeTypeInfo(finalDecl);
                        return 0;
                    }
                    freeTypeInfo(finalInit);
                    freeTypeInfo(finalDecl);
                }
            }

            // Finally register the variable symbol (typeNode now set or NULL)
            if (!defineSymbol(symbols, node->variable.name, SYM_VARIABLE, 
                             node->variable.type, node->line)) {
                return 0;
            }

            return 1;
        }

        case NODE_FUNCTION_DECL: {
            // Symbol should have been created during program pre-declaration; enter a new scope and register params
            if (!enterScope(symbols)) return 0;

            for (int i = 0; i < node->function.paramCount; i++) {
                ASTNode* p = node->function.params[i];
                if (p->type != NODE_VAR_DECL) continue;
                if (!defineSymbol(symbols, p->variable.name, SYM_PARAMETER, p->variable.type, p->line)) {
                    exitScope(symbols);
                    return 0;
                }
            }

            // Type check function body
            if (!typeCheck(node->function.body, symbols)) {
                exitScope(symbols);
                return 0;
            }

            exitScope(symbols);
            return 1;
        }
            
        case NODE_ASSIGN: {
            // Check if assignment target variable exists
            if (node->assignment.target->type != NODE_VARIABLE) {
                fprintf(stderr, "[line %d] Error: Invalid assignment target\n", node->line);
                return 0;
            }
            
            char* varName = node->assignment.target->varRef.name;
            Symbol* symbol = resolveSymbol(symbols, varName);
            if (!symbol) {
                fprintf(stderr, "[line %d] Error: Undefined variable '%s'\n", 
                        node->line, varName);
                return 0;
            }
            
            // Check type compatibility
            TypeInfo* targetType = getTypeInfo(symbol->typeNode, symbols);
            TypeInfo* valueType = getTypeInfo(node->assignment.value, symbols);
            
            if (!targetType || !valueType) {
                if (targetType) freeTypeInfo(targetType);
                if (valueType) freeTypeInfo(valueType);
                fprintf(stderr, "[line %d] Error: Cannot determine type\n", node->line);
                return 0;
            }
            
            if (!areTypesCompatible(targetType, valueType)) {
                fprintf(stderr, "[line %d] Error: Type mismatch in assignment\n", node->line);
                freeTypeInfo(targetType);
                freeTypeInfo(valueType);
                return 0;
            }
            
            freeTypeInfo(targetType);
            freeTypeInfo(valueType);
            return 1;
        }
            
        case NODE_RETURN_STMT: {
            // TODO: Check return type matches function declaration
            if (node->returnStmt.value) {
                return typeCheck(node->returnStmt.value, symbols);
            }
            return 1;
        }
            
        case NODE_BINARY_EXPR:
            // Type checking is already performed in getTypeInfo
            return getTypeInfo(node, symbols) != NULL;
            
        default:
            // Other node types skipped for now
            return 1;
    }
}

// ============ Debug functions ============

void printSymbolTable(SymbolTable* table) {
    if (!table) {
        printf("Symbol table is NULL\n");
        return;
    }
    
    printf("=== Symbol Table (depth=%d, count=%d) ===\n", 
           table->scopeDepth, table->count);
    
    for (int i = 0; i < table->capacity; i++) {
        Symbol* symbol = table->buckets[i];
        while (symbol) {
            const char* typeStr = "";
            switch (symbol->type) {
                case SYM_VARIABLE: typeStr = "variable"; break;
                case SYM_FUNCTION: typeStr = "function"; break;
                case SYM_PARAMETER: typeStr = "parameter"; break;
                case SYM_CLASS: typeStr = "class"; break;
            }
            
            printf("  %s: %s (line %d, depth %d)\n", 
                   symbol->name, typeStr, symbol->definedLine, symbol->scopeDepth);
            symbol = symbol->next;
        }
    }
}