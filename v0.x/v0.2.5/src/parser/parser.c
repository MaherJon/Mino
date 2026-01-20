// src/parser/parser.c - parser implementation (function call support)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "ast.h"

typedef struct {
    Lexer* lexer;
    Token current;
    Token previous;
    int hadError;
    int panicMode;
} Parser;

// ============ Error handling ============
static void errorAt(Parser* parser, Token* token, const char* message) {
    if (parser->panicMode) return;
    parser->panicMode = 1;
    
    fprintf(stderr, "[line %d] Error", token->line);
    
    if (token->type == TOKEN_EOF) {
        fprintf(stderr, " at end");
    } else if (token->type == TOKEN_ERROR) {
        // Nothing
    } else {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }
    
    fprintf(stderr, ": %s\n", message);
    parser->hadError = 1;
}

static void error(Parser* parser, const char* message) {
    errorAt(parser, &parser->previous, message);
}

static void errorAtCurrent(Parser* parser, const char* message) {
    errorAt(parser, &parser->current, message);
}

// ============ Token handling ============
static void advance(Parser* parser) {
    parser->previous = parser->current;
    parser->current = scanToken(parser->lexer);
}

static int check(Parser* parser, TokenType type) {
    return parser->current.type == type;
}

static int match(Parser* parser, TokenType type) {
    if (!check(parser, type)) return 0;
    advance(parser);
    return 1;
}

static TokenType peekType(Parser* parser) {
    return parser->current.type;
}

static void consume(Parser* parser, TokenType type, const char* message) {
    if (parser->current.type == type) {
        advance(parser);
        return;
    }
    errorAtCurrent(parser, message);
}

static void initParser(Parser* parser, Lexer* lexer) {
    parser->lexer = lexer;
    parser->hadError = 0;
    parser->panicMode = 0;
    advance(parser);
}

// ============ Helper functions ============
static char* copyString(const char* start, int length) {
    char* result = malloc(length + 1);
    memcpy(result, start, length);
    result[length] = '\0';
    return result;
}

// ============ Declarations ============
static ASTNode* expression(Parser* parser);
static ASTNode* varDeclaration(Parser* parser);

// ============ Expression parsing ============
// Parse function call; callee is the expression being called (variable or get expression)
static ASTNode* finishCall(Parser* parser, ASTNode* callee) {
    ASTNode** args = NULL;
    int argCount = 0;

    if (!match(parser, TOKEN_LEFT_PAREN)) {
        errorAtCurrent(parser, "Expect '(' after function name.");
        freeAST(callee);
        return NULL;
    }

    if (!check(parser, TOKEN_RIGHT_PAREN)) {
        do {
            ASTNode* arg = expression(parser);
            if (arg) {
                args = realloc(args, sizeof(ASTNode*) * (argCount + 1));
                args[argCount++] = arg;
            }
        } while (match(parser, TOKEN_COMMA));
    }

    consume(parser, TOKEN_RIGHT_PAREN, "Expect ')' after arguments.");

    return createCallNode(callee, args, argCount);
}

// Parse primary expressions: literals, identifiers, function calls
static ASTNode* primary(Parser* parser) {
    if (match(parser, TOKEN_TRUE) || match(parser, TOKEN_FALSE) || 
        match(parser, TOKEN_NULL) || match(parser, TOKEN_NUMBER)) {
        return createLiteralNode(parser->previous);
    }
    
    if (match(parser, TOKEN_IDENTIFIER)) {
        char* name = copyString(parser->previous.start, parser->previous.length);
        ASTNode* node = createVarRefNode(name);
        free(name);

        // Support dot access chains like sys.IO.print
        while (1) {
            if (match(parser, TOKEN_DOT)) {
                consume(parser, TOKEN_IDENTIFIER, "Expect member name after '.'.");
                char* member = copyString(parser->previous.start, parser->previous.length);
                ASTNode* getNode = createGetNode(node, member);
                free(member);
                node = getNode;
                continue;
            }

            // If a left parenthesis follows, this is a function call
            if (check(parser, TOKEN_LEFT_PAREN)) {
                return finishCall(parser, node);
            }

            break;
        }

        return node;
    }
    
    // Handle parenthesized expression
    if (match(parser, TOKEN_LEFT_PAREN)) {
        ASTNode* expr = expression(parser);
        consume(parser, TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
        return expr;
    }
    
    errorAtCurrent(parser, "Expect expression.");
    return NULL;
}

// Parse binary expressions
static ASTNode* binary(Parser* parser) {
    ASTNode* left = primary(parser);
    
    while (1) {
        if (match(parser, TOKEN_PLUS) || match(parser, TOKEN_MINUS) ||
            match(parser, TOKEN_STAR) || match(parser, TOKEN_SLASH)) {
            Token op = parser->previous;
            ASTNode* right = primary(parser);
            left = createBinaryNode(op, left, right);
        } else {
            break;
        }
    }
    
    return left;
}

static ASTNode* expression(Parser* parser) {
    return binary(parser);
}

// ============ Statement parsing ============
static ASTNode* expressionStatement(Parser* parser) {
    ASTNode* expr = expression(parser);
    consume(parser, TOKEN_SEMICOLON, "Expect ';' after expression.");
    return expr;
}

static ASTNode* returnStatement(Parser* parser) {
    ASTNode* value = NULL;
    
    if (!check(parser, TOKEN_SEMICOLON)) {
        value = expression(parser);
    }
    
    consume(parser, TOKEN_SEMICOLON, "Expect ';' after return value.");
    return createReturnNode(value);
}

static ASTNode* statement(Parser* parser) {
    if (match(parser, TOKEN_RETURN)) return returnStatement(parser);
    if (match(parser, TOKEN_LET) || match(parser, TOKEN_VAR)) return varDeclaration(parser);
    return expressionStatement(parser);
}

// ============ Declaration parsing ============
static ASTNode* varDeclaration(Parser* parser) {
    // 'let' or 'var' already matched
    consume(parser, TOKEN_IDENTIFIER, "Expect variable name.");
    char* name = copyString(parser->previous.start, parser->previous.length);
    
    ASTNode* typeNode = NULL;
    ASTNode* initializer = NULL;
    
    // Check for type annotation
    if (match(parser, TOKEN_COLON)) {
        // Format 1: with colon
        if (match(parser, TOKEN_INT) || match(parser, TOKEN_FLOAT) || 
            match(parser, TOKEN_BOOL) || match(parser, TOKEN_STRING_TYPE)) {
            typeNode = createLiteralNode(parser->previous);
        } else {
            errorAtCurrent(parser, "Expect type after :");
            free(name);
            return NULL;
        }
    } else if (peekType(parser) == TOKEN_INT || peekType(parser) == TOKEN_FLOAT || 
               peekType(parser) == TOKEN_BOOL || peekType(parser) == TOKEN_STRING_TYPE) {
        // Format 2: without colon (type before name)
        advance(parser);
        typeNode = createLiteralNode(parser->previous);
    }
    
    // Check for initializer expression
    if (match(parser, TOKEN_EQUAL)) {
        initializer = expression(parser);
    }
    
    consume(parser, TOKEN_SEMICOLON, "Expect ';' after variable declaration.");
    return createVarNode(name, typeNode, initializer);
}

static ASTNode* functionDeclaration(Parser* parser) {
    // TOKEN_FUNC already matched
    
    // Parse return type (optional)
    ASTNode* returnType = NULL;
    
    if (peekType(parser) == TOKEN_INT || peekType(parser) == TOKEN_FLOAT || 
        peekType(parser) == TOKEN_BOOL || peekType(parser) == TOKEN_STRING_TYPE ||
        peekType(parser) == TOKEN_VOID) {
        advance(parser);
        returnType = createLiteralNode(parser->previous);
    }
    
    // Parse function name
    consume(parser, TOKEN_IDENTIFIER, "Expect function name.");
    char* name = copyString(parser->previous.start, parser->previous.length);
    
    // Parse parameter list
    consume(parser, TOKEN_LEFT_PAREN, "Expect '(' after function name.");
    
    ASTNode** params = NULL;
    int paramCount = 0;
    
    if (!check(parser, TOKEN_RIGHT_PAREN)) {
        do {
            // Parse parameter type
            if (!(match(parser, TOKEN_INT) || match(parser, TOKEN_FLOAT) || 
                  match(parser, TOKEN_BOOL) || match(parser, TOKEN_STRING_TYPE))) {
                errorAtCurrent(parser, "Expect parameter type.");
                break;
            }
            Token paramType = parser->previous;
            
            // Parse parameter name
            consume(parser, TOKEN_IDENTIFIER, "Expect parameter name.");
            char* paramName = copyString(parser->previous.start, parser->previous.length);
            
            // Add to parameter list
            params = realloc(params, sizeof(ASTNode*) * (paramCount + 1));
            ASTNode* paramTypeNode = createLiteralNode(paramType);
            ASTNode* paramNode = createVarNode(paramName, paramTypeNode, NULL);
            params[paramCount++] = paramNode;
            
        } while (match(parser, TOKEN_COMMA));
    }
    
    consume(parser, TOKEN_RIGHT_PAREN, "Expect ')' after parameters.");
    
    // Parse function body
    consume(parser, TOKEN_LEFT_BRACE, "Expect '{' before function body.");
    
    ASTNode** bodyStatements = NULL;
    int bodyCount = 0;
    
    while (!check(parser, TOKEN_RIGHT_BRACE) && !check(parser, TOKEN_EOF)) {
        ASTNode* stmt = statement(parser);
        if (stmt) {
            bodyStatements = realloc(bodyStatements, sizeof(ASTNode*) * (bodyCount + 1));
            bodyStatements[bodyCount++] = stmt;
        }
    }
    
    consume(parser, TOKEN_RIGHT_BRACE, "Expect '}' after function body.");
    
    ASTNode* body = createProgramNode(bodyStatements, bodyCount);
    return createFunctionNode(name, params, paramCount, returnType, body);
}

static ASTNode* includeDeclaration(Parser* parser) {
    // For tests, skip include blocks
    while (!check(parser, TOKEN_EOF) && parser->current.type != TOKEN_FUNC) {
        advance(parser);
    }
    return NULL;
}

static ASTNode* declaration(Parser* parser) {
    if (check(parser, TOKEN_INCLUDE)) {
        advance(parser);
        return includeDeclaration(parser);
    }
    
    if (match(parser, TOKEN_FUNC)) {
        return functionDeclaration(parser);
    }
    
    if (match(parser, TOKEN_LET) || match(parser, TOKEN_VAR)) {
        return varDeclaration(parser);
    }
    
    return statement(parser);
}

// ============ Main parser ============
ASTNode* parse(const char* source) {
    Lexer lexer;
    initLexer(&lexer, source);
    
    Parser parser;
    initParser(&parser, &lexer);
    
    ASTNode** statements = NULL;
    int statementCount = 0;
    
    while (!check(&parser, TOKEN_EOF)) {
        ASTNode* stmt = declaration(&parser);
        if (stmt) {
            statements = realloc(statements, sizeof(ASTNode*) * (statementCount + 1));
            statements[statementCount++] = stmt;
        }
    }
    
    if (parser.hadError) {
        for (int i = 0; i < statementCount; i++) {
            freeAST(statements[i]);
        }
        free(statements);
        return NULL;
    }
    
    return createProgramNode(statements, statementCount);
}