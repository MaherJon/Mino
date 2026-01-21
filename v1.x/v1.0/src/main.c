// src/main.c - driver for the Mino compiler
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lexer.h>
#include <ast.h>
#include <parser.h>
#include <semantic.h>

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

// forward declaration for helper below
static void getOutputPath(const char* filename, char* outPath, size_t outSize);

static void testLexer(const char* source) {
    Lexer lexer;
    initLexer(&lexer, source);
    
    printf("=== Tokenizing ===\n");
    
    int tokenCount = 0;
    while (1) {
        Token token = scanToken(&lexer);
        tokenCount++;
        
        printf("Line %d: ", token.line);
        
        switch (token.type) {
            // Single-character tokens
            case TOKEN_LEFT_PAREN: printf("("); break;
            case TOKEN_RIGHT_PAREN: printf(")"); break;
            case TOKEN_LEFT_BRACE: printf("{"); break;
            case TOKEN_RIGHT_BRACE: printf("}"); break;
            case TOKEN_SEMICOLON: printf(";"); break;
            case TOKEN_COMMA: printf(","); break;
            case TOKEN_DOT: printf("."); break;
            case TOKEN_COLON: printf(":"); break;
            case TOKEN_QUESTION: printf("?"); break;
            
            // Operators
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
            
            // Literals
            case TOKEN_IDENTIFIER: 
                printf("IDENTIFIER '%.*s'", token.length, token.start);
                break;
            case TOKEN_NUMBER:
                printf("NUMBER '%.*s'", token.length, token.start);
                break;
            case TOKEN_STRING:
                printf("STRING '%.*s'", token.length, token.start);
                break;
            
            // Keywords
            case TOKEN_FUNC: printf("func"); break;
            case TOKEN_CLASS: printf("class"); break;
            case TOKEN_LET: printf("let"); break;
            case TOKEN_VAR: printf("var"); break;
            case TOKEN_IF: printf("if"); break;
            case TOKEN_ELSE: printf("else"); break;
            case TOKEN_WHILE: printf("while"); break;
            case TOKEN_RETURN: printf("return"); break;
            case TOKEN_TRUE: printf("true"); break;
            case TOKEN_FALSE: printf("false"); break;
            case TOKEN_NULL: printf("null"); break;
            case TOKEN_INCLUDE: printf("#include"); break;
            
            // Type keywords
            case TOKEN_INT: printf("int"); break;
            case TOKEN_FLOAT: printf("float"); break;
            case TOKEN_BOOL: printf("bool"); break;
            case TOKEN_STRING_TYPE: printf("string"); break;
            case TOKEN_VOID: printf("void"); break;
            
            // Special
            case TOKEN_EOF: printf("EOF"); break;
            case TOKEN_ERROR:
                printf("ERROR: %.*s", token.length, token.start);
                break;
            case TOKEN_ARROW:
                printf("->"); break;
                
            default:
                printf("TOKEN(%d)", token.type);
        }
        printf("\n");
        
        if (token.type == TOKEN_ERROR || token.type == TOKEN_EOF) break;
    }
    
    printf("Total tokens: %d\n", tokenCount);
}

static void testParser(const char* source) {
    printf("=== Parsing ===\n");
    
    ASTNode* ast = parse(source);
    if (!ast) {
        printf("Parse failed!\n");
        return;
    }
    
    printf("Parse successful!\n");
    printf("\n=== AST Structure ===\n");
    printAST(ast, 0);
    
    // Semantic analysis test
    printf("\n=== Semantic Analysis ===\n");
    SymbolTable* symbols = createSymbolTable();
    
    if (typeCheck(ast, symbols)) {
        printf("Type checking passed!\n");
        printSymbolTable(symbols);
    } else {
        printf("Type checking failed!\n");
    }
    
    freeSymbolTable(symbols);
    freeAST(ast);
}

static void compileFile(const char* filename) {
    char* source = readFile(filename);
    
    printf("Compiling: %s\n", filename);
    printf("Source size: %zu bytes\n", strlen(source));
    
    // Test lexer
    testLexer(source);
    
    // Parse
    ASTNode* ast = parse(source);
    if (!ast) {
        fprintf(stderr, "Parse failed, aborting.\n");
        free(source);
        return;
    }

    printf("Parse successful!\n\n");
    printAST(ast, 0);

    // Semantic analysis
    printf("\n=== Semantic Analysis ===\n");
    SymbolTable* symbols = createSymbolTable();
    if (!typeCheck(ast, symbols)) {
        fprintf(stderr, "Type checking failed, aborting.\n");
        freeSymbolTable(symbols);
        freeAST(ast);
        free(source);
        return;
    }
    printf("Type checking passed!\n");

    // Code generation: generate executable
    printf("\n=== Code Generation ===\n");
    char outExe[256];
    getOutputPath(filename, outExe, sizeof(outExe));

    // Ensure runtime library/object exists; if not, build it via Makefile
    FILE* fa = fopen("lib/minolib/libminosys.a", "r");
    if (!fa) {
        FILE* fo = fopen("lib/minolib/System/System.o", "r");
        if (!fo) {
            printf("Runtime not found, building runtime...\n");
            int rc = system("make runtime");
            if (rc != 0) fprintf(stderr, "Warning: `make runtime` failed (rc=%d)\n", rc);
        } else {
            fclose(fo);
        }
    } else {
        fclose(fa);
    }

    extern int codegen_generateExecutable(ASTNode* ast, const char* outPath);
    if (codegen_generateExecutable(ast, outExe) == 0) {
        printf("Generated executable: %s\n", outExe);
    } else {
        fprintf(stderr, "Code generation failed.\n");
    }

    freeSymbolTable(symbols);
    freeAST(ast);
    
    free(source);
}

int main(int argc, char** argv) {
    // Build runtime helper objects: bin/minoc --build-runtime
    if (argc == 2 && strcmp(argv[1], "--build-runtime") == 0) {
        printf("Building runtime object...\n");
        int rc = system("gcc -c -I./include -o lib/minolib/System/System.o lib/minolib/System/System.c");
        if (rc == 0) printf("Built: lib/minolib/System/System.o\n");
        else fprintf(stderr, "Runtime build failed (rc=%d)\n", rc);
        return rc;
    }

    // Build static runtime archive: bin/minoc --build-runtime-static
    if (argc == 2 && strcmp(argv[1], "--build-runtime-static") == 0) {
        printf("Building static runtime archive...\n");
        int rc1 = system("gcc -c -I./include -o lib/minolib/System/System.o lib/minolib/System/System.c");
        if (rc1 != 0) { fprintf(stderr, "Compile runtime failed (rc=%d)\n", rc1); return rc1; }
        int rc2 = system("ar rcs lib/minolib/libminosys.a lib/minolib/System/System.o");
        if (rc2 == 0) printf("Built: lib/minolib/libminosys.a\n");
        else fprintf(stderr, "Archive creation failed (rc=%d)\n", rc2);
        return rc2;
    }
    if (argc == 1) {
        printf("Mino Compiler v0.2.5\n");
        printf("Usage: minoc <filename.mino|filename.mi>\n");
        printf("       minoc --test <test_string>\n");
        printf("       minoc --lex <filename>\n");
        printf("       minoc --parse <filename>\n");
        return 1;
    }
    
    if (argc == 3 && strcmp(argv[1], "--test") == 0) {
        testLexer(argv[2]);
        testParser(argv[2]);
        return 0;
    }
    
    if (argc == 3 && strcmp(argv[1], "--lex") == 0) {
        char* source = readFile(argv[2]);
        testLexer(source);
        free(source);
        return 0;
    }
    
    if (argc == 3 && strcmp(argv[1], "--parse") == 0) {
        char* source = readFile(argv[2]);
        testParser(source);
        free(source);
        return 0;
    }
    
    // Compile file normally
    compileFile(argv[1]);
    
    return 0;
}

// Helper: generate output executable path by stripping known extensions
// Supported source extensions: .mino, .mi
static void getOutputPath(const char* filename, char* outPath, size_t outSize) {
    // copy filename so we can modify
    char tmp[512];
    strncpy(tmp, filename, sizeof(tmp)-1);
    tmp[sizeof(tmp)-1] = '\0';

    // find last path separator and last dot
    char* lastSlash = strrchr(tmp, '/');
    char* lastDot = strrchr(tmp, '.');
    if (lastDot != NULL) {
        // ensure dot is after last slash (i.e., in basename)
        if (lastSlash == NULL || lastDot > lastSlash) {
            if (strcmp(lastDot, ".mino") == 0 || strcmp(lastDot, ".mi") == 0) {
                *lastDot = '\0';
            }
        }
    }

    snprintf(outPath, outSize, "%s.out", tmp);
}