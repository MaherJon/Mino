// src/main.c -- Mino Compiler v0.1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lexer.h>
#include <ast.h>

static char* readFile(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        fprintf(stderr, "Could not open file \"%s\".\n", filename);
        exit(74);
    }
    
    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);
    
    char* buffer = (char*)malloc(fileSize + 1);
    if (buffer == NULL) {
        fprintf(stderr, "Not enough memory to read \"%s\".\n", filename);
        exit(74);
    }
    
    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    if (bytesRead < fileSize) {
        fprintf(stderr, "Could not read file \"%s\".\n", filename);
        exit(74);
    }
    
    buffer[bytesRead] = '\0';
    
    fclose(file);
    return buffer;
}

static void testLexer(const char* source) {
    Lexer lexer;
    initLexer(&lexer, source);
    
    printf("=== Tokenizing ===\n");
    
    int tokenCount = 0;
    while (1) {
        Token token = scanToken(&lexer);
        tokenCount++;
        
        // 打印token信息
        printf("Line %d: ", token.line);
        
        switch (token.type) {
            // 单字符 tokens
            case TOKEN_LEFT_PAREN: printf("("); break;
            case TOKEN_RIGHT_PAREN: printf(")"); break;
            case TOKEN_LEFT_BRACE: printf("{"); break;
            case TOKEN_RIGHT_BRACE: printf("}"); break;
            case TOKEN_LEFT_BRACKET: printf("["); break;
            case TOKEN_RIGHT_BRACKET: printf("]"); break;
            case TOKEN_SEMICOLON: printf(";"); break;
            case TOKEN_COMMA: printf(","); break;
            case TOKEN_DOT: printf("."); break;
            case TOKEN_COLON: printf(":"); break;
            case TOKEN_QUESTION: printf("?"); break;
            
            // 运算符
            case TOKEN_PLUS: printf("+"); break;
            case TOKEN_MINUS: printf("-"); break;
            case TOKEN_STAR: printf("*"); break;
            case TOKEN_SLASH: printf("/"); break;
            case TOKEN_PERCENT: printf("%%"); break;
            case TOKEN_BANG: printf("!"); break;
            case TOKEN_BANG_EQUAL: printf("!="); break;
            case TOKEN_EQUAL: printf("="); break;
            case TOKEN_EQUAL_EQUAL: printf("=="); break;
            case TOKEN_GREATER: printf(">"); break;
            case TOKEN_GREATER_EQUAL: printf(">="); break;
            case TOKEN_LESS: printf("<"); break;
            case TOKEN_LESS_EQUAL: printf("<="); break;
            case TOKEN_AMPERSAND: printf("&"); break;
            case TOKEN_AMPERSAND_AMPERSAND: printf("&&"); break;
            case TOKEN_PIPE: printf("|"); break;
            case TOKEN_PIPE_PIPE: printf("||"); break;
            
            // 字面量
            case TOKEN_IDENTIFIER: 
                printf("IDENTIFIER(%.*s)", token.length, token.start);
                break;
            case TOKEN_NUMBER:
                printf("NUMBER(%.*s)", token.length, token.start);
                break;
            case TOKEN_STRING:
                printf("STRING(%.*s)", token.length, token.start);
                break;
            
            // 关键字
            case TOKEN_FUNC: printf("func"); break;
            case TOKEN_CLASS: printf("class"); break;
            case TOKEN_LET: printf("let"); break;
            case TOKEN_VAR: printf("var"); break;
            case TOKEN_IF: printf("if"); break;
            case TOKEN_ELSE: printf("else"); break;
            case TOKEN_WHILE: printf("while"); break;
            case TOKEN_FOR: printf("for"); break;
            case TOKEN_RETURN: printf("return"); break;
            case TOKEN_TRUE: printf("true"); break;
            case TOKEN_FALSE: printf("false"); break;
            case TOKEN_NULL: printf("null"); break;
            case TOKEN_AND: printf("and"); break;
            case TOKEN_OR: printf("or"); break;
            case TOKEN_NEW: printf("new"); break;
            case TOKEN_THIS: printf("this"); break;
            case TOKEN_SUPER: printf("super"); break;
            case TOKEN_INCLUDE: printf("#include"); break;
            
            // 类型关键字
            case TOKEN_INT: printf("int"); break;
            case TOKEN_FLOAT: printf("float"); break;
            case TOKEN_BOOL: printf("bool"); break;
            case TOKEN_STRING_TYPE: printf("String"); break;
            case TOKEN_VOID: printf("void"); break;
            
            // 特殊
            case TOKEN_EOF: printf("EOF"); break;
            case TOKEN_ERROR:
                printf("ERROR: %.*s", token.length, token.start);
                break;
                
            default:
                printf("UNKNOWN_TOKEN(%d)", token.type);  // 这行应该很少出现
        }
        printf("\n");
        
        if (token.type == TOKEN_ERROR || token.type == TOKEN_EOF) break;
    }
}

int main(int argc, char** argv) {
    if (argc == 1) {
        printf("Usage: minoc <filename.mino>\n");
        printf("       minoc --test <test_string>\n");
        return 1;
    }
    
    if (argc == 3 && strcmp(argv[1], "--test") == 0) {
        testLexer(argv[2]);
        return 0;
    }
    
    char* source = readFile(argv[1]);
    
    printf("Compiling: %s\n", argv[1]);
    printf("Source size: %zu bytes\n", strlen(source));
    
    testLexer(source);
    
    // TODO: 解析和代码生成
    // ASTNode* ast = parse(source);
    // generateCode(ast);
    
    free(source);
    return 0;
}