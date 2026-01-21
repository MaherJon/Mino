#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "tokens.h"

// src/lexer/lexer.c - lexer implementation
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "tokens.h"

typedef struct {
    const char* start; 
    const char* current;
    int line;
} Lexer;

// ============ Forward declarations ============
static int isAtEnd(Lexer* lexer);
static char advance(Lexer* lexer);
static char peek(Lexer* lexer);
static char peekNext(Lexer* lexer);
static int match(Lexer* lexer, char expected);
static void skipWhitespace(Lexer* lexer);
static Token makeToken(Lexer* lexer, TokenType type);
static Token errorToken(Lexer* lexer, const char* message);
static TokenType checkKeyword(Lexer* lexer, int start, int length,
                             const char* rest, TokenType type);
static TokenType identifierType(Lexer* lexer);
static Token identifier(Lexer* lexer);
static Token number(Lexer* lexer);
static Token string(Lexer* lexer);

// ============ Implementation ============

void initLexer(Lexer* lexer, const char* source) {
    lexer->start = source;
    lexer->current = source;
    lexer->line = 1;
}

static int isAtEnd(Lexer* lexer) {
    return *lexer->current == '\0';
}

static char advance(Lexer* lexer) {
    lexer->current++;
    return lexer->current[-1];
}

static char peek(Lexer* lexer) {
    return *lexer->current;
}

static char peekNext(Lexer* lexer) {
    if (isAtEnd(lexer)) return '\0';
    return lexer->current[1];
}

static int match(Lexer* lexer, char expected) {
    if (isAtEnd(lexer)) return 0;
    if (*lexer->current != expected) return 0;
    lexer->current++;
    return 1;
}

static void skipWhitespace(Lexer* lexer) {
    while (1) {
        char c = peek(lexer);
        switch (c) {
            case ' ':
            case '\t':
            case '\r':
                advance(lexer);
                break;
            case '\n':
                lexer->line++;
                advance(lexer);
                break;
            case '/':
                if (peekNext(lexer) == '/') {
                    while (peek(lexer) != '\n' && !isAtEnd(lexer)) {
                        advance(lexer);
                    }
                } else if (peekNext(lexer) == '*') {
                    advance(lexer); // skip '/'
                    advance(lexer); // skip '*'
                    while (!(peek(lexer) == '*' && peekNext(lexer) == '/') && !isAtEnd(lexer)) {
                        if (peek(lexer) == '\n') lexer->line++;
                        advance(lexer);
                    }
                    if (!isAtEnd(lexer)) {
                        advance(lexer); // skip '*'
                        advance(lexer); // skip '/'
                    }
                } else {
                    return;
                }
                break;
            default:
                return;
        }
    }
}

static Token makeToken(Lexer* lexer, TokenType type) {
    Token token;
    token.type = type;
    token.start = lexer->start;
    token.length = (int)(lexer->current - lexer->start);
    token.line = lexer->line;
    return token;
}

static Token errorToken(Lexer* lexer, const char* message) {
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = lexer->line;
    return token;
}

static TokenType checkKeyword(Lexer* lexer, int start, int length,
                             const char* rest, TokenType type) {
    if (lexer->current - lexer->start == start + length &&
        memcmp(lexer->start + start, rest, length) == 0) {
        return type;
    }
    return TOKEN_IDENTIFIER;
}

static TokenType identifierType(Lexer* lexer) {
    switch (lexer->start[0]) {
        case 'a': 
            if (lexer->current - lexer->start > 1 && lexer->start[1] == 'n' &&
                lexer->current - lexer->start == 3 && lexer->start[2] == 'd') {
                return TOKEN_AND;
            }
            break;
        case 'b': 
            if (lexer->current - lexer->start == 4) {
                if (memcmp(lexer->start + 1, "ool", 3) == 0) return TOKEN_BOOL;
            }
            break;
        case 'c': 
            if (lexer->current - lexer->start == 5) {
                if (memcmp(lexer->start + 1, "lass", 4) == 0) return TOKEN_CLASS;
            }
            break;
        case 'e': 
            if (lexer->current - lexer->start == 4) {
                if (memcmp(lexer->start + 1, "lse", 3) == 0) return TOKEN_ELSE;
            }
            break;
        case 'f': 
            if (lexer->current - lexer->start > 1) {
                if (lexer->current - lexer->start == 5) {
                    if (memcmp(lexer->start + 1, "alse", 4) == 0) return TOKEN_FALSE;
                    if (memcmp(lexer->start + 1, "loat", 4) == 0) return TOKEN_FLOAT;
                }
                if (lexer->current - lexer->start == 3) {
                    if (memcmp(lexer->start + 1, "or", 2) == 0) return TOKEN_FOR;
                }
                if (lexer->current - lexer->start == 4) {
                    if (memcmp(lexer->start + 1, "unc", 3) == 0) return TOKEN_FUNC;
                }
            }
            break;
        case 'i': 
            if (lexer->current - lexer->start == 2) {
                if (memcmp(lexer->start + 1, "f", 1) == 0) return TOKEN_IF;
            }
            if (lexer->current - lexer->start == 3) {
                if (memcmp(lexer->start + 1, "nt", 2) == 0) return TOKEN_INT;
            }
            break;
        case 'l': 
            if (lexer->current - lexer->start == 3) {
                if (memcmp(lexer->start + 1, "et", 2) == 0) return TOKEN_LET;
            }
            break;
        case 'n': 
            if (lexer->current - lexer->start > 1) {
                if (lexer->current - lexer->start == 3) {
                    if (memcmp(lexer->start + 1, "ew", 2) == 0) return TOKEN_NEW;
                }
                if (lexer->current - lexer->start == 4) {
                    if (memcmp(lexer->start + 1, "ull", 3) == 0) return TOKEN_NULL;
                }
            }
            break;
        case 'o': 
            if (lexer->current - lexer->start == 2) {
                if (memcmp(lexer->start + 1, "r", 1) == 0) return TOKEN_OR;
            }
            break;
        case 'r': 
            if (lexer->current - lexer->start == 6) {
                if (memcmp(lexer->start + 1, "eturn", 5) == 0) return TOKEN_RETURN;
            }
            break;
        case 's': 
            if (lexer->current - lexer->start > 1) {
                if (lexer->current - lexer->start == 6) {
                    if (memcmp(lexer->start + 1, "tring", 5) == 0) return TOKEN_STRING_TYPE;
                    if (memcmp(lexer->start + 1, "uper", 4) == 0) return TOKEN_SUPER;
                }
                if (lexer->current - lexer->start == 4) {
                    if (memcmp(lexer->start + 1, "can", 3) == 0) return TOKEN_IDENTIFIER; // not a keyword
                }
            }
            break;
        case 't': 
            if (lexer->current - lexer->start > 1) {
                if (lexer->current - lexer->start == 4) {
                    if (memcmp(lexer->start + 1, "his", 3) == 0) return TOKEN_THIS;
                    if (memcmp(lexer->start + 1, "rue", 3) == 0) return TOKEN_TRUE;
                }
            }
            break;
        case 'v': 
            if (lexer->current - lexer->start > 1) {
                if (lexer->current - lexer->start == 3) {
                    if (memcmp(lexer->start + 1, "ar", 2) == 0) return TOKEN_VAR;
                }
                if (lexer->current - lexer->start == 4) {
                    if (memcmp(lexer->start + 1, "oid", 3) == 0) return TOKEN_VOID;
                }
            }
            break;
        case 'w': 
            if (lexer->current - lexer->start == 5) {
                if (memcmp(lexer->start + 1, "hile", 4) == 0) return TOKEN_WHILE;
            }
            break;
    }
    return TOKEN_IDENTIFIER;
}

static Token identifier(Lexer* lexer) {
    while (isalpha(peek(lexer)) || isdigit(peek(lexer)) || peek(lexer) == '_') {
        advance(lexer);
    }
    return makeToken(lexer, identifierType(lexer));
}

static Token number(Lexer* lexer) {
    while (isdigit(peek(lexer))) advance(lexer);
    
    if (peek(lexer) == '.' && isdigit(peekNext(lexer))) {
        advance(lexer); // skip decimal point
        
        while (isdigit(peek(lexer))) advance(lexer);
    }
    
    return makeToken(lexer, TOKEN_NUMBER);
}

static Token string(Lexer* lexer) {
    while (peek(lexer) != '"' && !isAtEnd(lexer)) {
        if (peek(lexer) == '\n') lexer->line++;
        advance(lexer);
    }
    
    if (isAtEnd(lexer)) return errorToken(lexer, "Unterminated string.");
    
    advance(lexer);
    return makeToken(lexer, TOKEN_STRING);
}

// ============ Main scanner ============
Token scanToken(Lexer* lexer) {
    skipWhitespace(lexer);
    lexer->start = lexer->current;
    
    if (isAtEnd(lexer)) return makeToken(lexer, TOKEN_EOF);
    
    char c = advance(lexer);
    
    if (c == '#') {
        while (isalpha(peek(lexer))) {
            advance(lexer);
        }
        
        int length = lexer->current - lexer->start;
        if (length == 8 && memcmp(lexer->start, "#include", 8) == 0) {
            return makeToken(lexer, TOKEN_INCLUDE);
        }
        
        return errorToken(lexer, "Unknown preprocessor directive");
    }
    
    if (isalpha(c) || c == '_') return identifier(lexer);
    if (isdigit(c)) return number(lexer);
    
    switch (c) {
        case '(': return makeToken(lexer, TOKEN_LEFT_PAREN);
        case ')': return makeToken(lexer, TOKEN_RIGHT_PAREN);
        case '{': return makeToken(lexer, TOKEN_LEFT_BRACE);
        case '}': return makeToken(lexer, TOKEN_RIGHT_BRACE);
        case '[': return makeToken(lexer, TOKEN_LEFT_BRACKET);
        case ']': return makeToken(lexer, TOKEN_RIGHT_BRACKET);
        case ';': return makeToken(lexer, TOKEN_SEMICOLON);
        case ',': return makeToken(lexer, TOKEN_COMMA);
        case '.': return makeToken(lexer, TOKEN_DOT);
        case ':': return makeToken(lexer, TOKEN_COLON);
        case '?': return makeToken(lexer, TOKEN_QUESTION);
        
        case '+': return makeToken(lexer, TOKEN_PLUS);
        case '/': return makeToken(lexer, TOKEN_SLASH);
        case '*': return makeToken(lexer, TOKEN_STAR);
        case '%': return makeToken(lexer, TOKEN_PERCENT);
        
        case '!':
            return makeToken(lexer, match(lexer, '=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
        case '=':
            return makeToken(lexer, match(lexer, '=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
        case '<':
            return makeToken(lexer, match(lexer, '=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
        case '>':
            return makeToken(lexer, match(lexer, '=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
        
        case '-':
            if (match(lexer, '>')) {
                return makeToken(lexer, TOKEN_ARROW);
            }
            return makeToken(lexer, TOKEN_MINUS);
        
        case '&':
            return makeToken(lexer, match(lexer, '&') ? TOKEN_AMPERSAND_AMPERSAND : TOKEN_AMPERSAND);
        case '|':
            return makeToken(lexer, match(lexer, '|') ? TOKEN_PIPE_PIPE : TOKEN_PIPE);
        
        case '"': return string(lexer);
    }
    
    return errorToken(lexer, "Unexpected character.");
}