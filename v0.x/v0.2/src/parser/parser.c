// src/parser/parser.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lexer.h>
#include <ast.h>

typedef struct {
    Lexer* lexer;
    Token current;
    Token previous;
    int hadError;
    int panicMode;
} Parser;

// Err处理
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

static void advance(Parser* parser) {
    parser->previous = parser->current;
    
    while (1) {
        parser->current = scanToken(parser->lexer);
        if (parser->current.type != TOKEN_ERROR) break;
        
        errorAtCurrent(parser, parser->current.start);
    }
}

static int check(Parser* parser, TokenType type) {
    return parser->current.type == type;
}

static int match(Parser* parser, TokenType type) {
    if (!check(parser, type)) return 0;
    advance(parser);
    return 1;
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

static char* copyString(const char* start, int length) {
    char* result = malloc(length + 1);
    memcpy(result, start, length);
    result[length] = '\0';
    return result;
}

ASTNode* parse(const char* source) {
    Lexer lexer;
    initLexer(&lexer, source);
    
    Parser parser;
    initParser(&parser, &lexer);
    
    ASTNode** statements = NULL;
    return createProgramNode(statements, 0);
}