#ifndef MINO_SEMANTIC_H
#define MINO_SEMANTIC_H

#include "ast.h"

// Forward declarations
typedef struct Symbol Symbol;
typedef struct SymbolTable SymbolTable;

// Symbol types
// ============ Symbol types =================
typedef enum {
    SYM_VARIABLE,
    SYM_FUNCTION,
    SYM_PARAMETER,
    SYM_CLASS
} SymbolType;

// Symbol structure
struct Symbol {
    char* name;
    SymbolType type;
    ASTNode* typeNode;      // type information
    int scopeDepth;         // scope depth
    int definedLine;        // definition line
    Symbol* next;           // next in linked list
};

// Symbol table structure
struct SymbolTable {
    Symbol** buckets;       // hash buckets
    int capacity;           // capacity
    int count;              // number of symbols
    int scopeDepth;         // current scope depth
};

// Type information
typedef struct TypeInfo {
    char* name;             // type name
    int size;               // size (bytes)
    int isArray;            // is array
    int isPrimitive;        // is primitive type
    struct TypeInfo* base;  // base type
} TypeInfo;

// Function declarations
SymbolTable* createSymbolTable();
void freeSymbolTable(SymbolTable* table);
int enterScope(SymbolTable* table);
int exitScope(SymbolTable* table);
int defineSymbol(SymbolTable* table, const char* name, SymbolType type, 
                 ASTNode* typeNode, int line);
Symbol* resolveSymbol(SymbolTable* table, const char* name);
int typeCheck(ASTNode* node, SymbolTable* symbols);
TypeInfo* getTypeInfo(ASTNode* node, SymbolTable* symbols);
int areTypesCompatible(TypeInfo* t1, TypeInfo* t2);

// Debug function declaration (must come after type definitions)
void printSymbolTable(SymbolTable* table);

#endif