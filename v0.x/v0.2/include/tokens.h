// include/tokens.h
#ifndef MINO_TOKENS_H
#define MINO_TOKENS_H

typedef enum {
    // 单字符token
    TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN,    // ( )
    TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,    // { }
    TOKEN_LEFT_BRACKET, TOKEN_RIGHT_BRACKET,// [ ]
    TOKEN_COMMA, TOKEN_DOT, TOKEN_SEMICOLON, // , . ;
    TOKEN_COLON, TOKEN_QUESTION,            // : ?
    
    // 运算符
    TOKEN_PLUS, TOKEN_MINUS,                // + -
    TOKEN_STAR, TOKEN_SLASH, TOKEN_PERCENT, // * / %
    TOKEN_BANG, TOKEN_BANG_EQUAL,           // ! !=
    TOKEN_EQUAL, TOKEN_EQUAL_EQUAL,         // = ==
    TOKEN_GREATER, TOKEN_GREATER_EQUAL,     // > >=
    TOKEN_LESS, TOKEN_LESS_EQUAL,           // < <=
    TOKEN_AMPERSAND, TOKEN_AMPERSAND_AMPERSAND, // & &&
    TOKEN_PIPE, TOKEN_PIPE_PIPE,            // | ||
    
    // 字面量
    TOKEN_IDENTIFIER, TOKEN_STRING, TOKEN_NUMBER,
    
    // 关键字
    TOKEN_FUNC, TOKEN_CLASS, TOKEN_LET, TOKEN_VAR,
    TOKEN_IF, TOKEN_ELSE, TOKEN_WHILE, TOKEN_FOR,
    TOKEN_RETURN, TOKEN_TRUE, TOKEN_FALSE, TOKEN_NULL,
    TOKEN_AND, TOKEN_OR, TOKEN_NEW, TOKEN_THIS,
    TOKEN_SUPER, TOKEN_INCLUDE,
    
    // 类型关键字
    TOKEN_INT, TOKEN_FLOAT, TOKEN_BOOL, TOKEN_STRING_TYPE,
    TOKEN_VOID,

    TOKEN_HASH,           // #
    TOKEN_HASH_INCLUDE,   // #include
    
    // 特殊
    TOKEN_ERROR, TOKEN_EOF
} TokenType;

typedef struct {
    TokenType type;
    const char* start;
    int length; 
    int line; 
} Token;

#endif