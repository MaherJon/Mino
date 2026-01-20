// include/lexer.h
#ifndef MINO_LEXER_H
#define MINO_LEXER_H

#include <tokens.h>

typedef struct {
    const char* start;
    const char* current;
    int line;
} Lexer;

void initLexer(Lexer* lexer, const char* source);

Token scanToken(Lexer* lexer);

#endif