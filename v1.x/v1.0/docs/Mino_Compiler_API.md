# Mino Compiler API / AST Reference

This document summarizes the public AST structures and core API functions exposed in the `include/` headers.

## Files referenced

- `include/ast.h`
- `include/tokens.h`
- `include/lexer.h`
- `include/parser.h`
- `include/semantic.h`

## Tokens

Type: `TokenType` (see `include/tokens.h`)

- Single-character tokens: `TOKEN_LEFT_PAREN`, `TOKEN_RIGHT_PAREN`, `TOKEN_LEFT_BRACE`, `TOKEN_RIGHT_BRACE`, etc.
- Operators: `TOKEN_PLUS`, `TOKEN_MINUS`, `TOKEN_STAR`, `TOKEN_SLASH`, `TOKEN_PERCENT`, `TOKEN_BANG`, `TOKEN_EQUAL`, etc.
- Literals: `TOKEN_IDENTIFIER`, `TOKEN_STRING`, `TOKEN_NUMBER`.
- Keywords: `TOKEN_FUNC`, `TOKEN_CLASS`, `TOKEN_LET`, `TOKEN_VAR`, `TOKEN_IF`, `TOKEN_ELSE`, `TOKEN_WHILE`, `TOKEN_RETURN`, etc.
- Type keywords: `TOKEN_INT`, `TOKEN_FLOAT`, `TOKEN_BOOL`, `TOKEN_STRING_TYPE`, `TOKEN_VOID`.

Struct: `Token`

- `TokenType type` — token kind
- `const char* start` — pointer to token start in source
- `int length` — token text length
- `int line` — source line number

## AST (include/ast.h)

Enum: `NodeType` — enumerates node kinds, including:

- `NODE_PROGRAM` — root program node
- `NODE_FUNCTION_DECL` — function declaration
- `NODE_CLASS_DECL` — class declaration
- `NODE_VAR_DECL` — variable declaration
- `NODE_EXPR_STMT`, `NODE_RETURN_STMT`, `NODE_IF_STMT`, `NODE_WHILE_STMT`, `NODE_BLOCK_STMT`
- Expression nodes: `NODE_BINARY_EXPR`, `NODE_UNARY_EXPR`, `NODE_CALL_EXPR`, `NODE_GET_EXPR`, `NODE_SET_EXPR`, `NODE_LITERAL`, `NODE_VARIABLE`, `NODE_ASSIGN`
- `NODE_INCLUDE` — include directive

Struct: `ASTNode`

- `NodeType type` — node kind
- `int line` — source line
- `union` — payload depends on node type, includes:
  - Program: `ASTNode** statements; int count;`
  - Function: `char* name; ASTNode** params; int paramCount; ASTNode* returnType; ASTNode* body;`
  - Variable: `char* name; ASTNode* type; ASTNode* initializer;`
  - Literal: `Token token;`
  - VarRef: `char* name;`
  - Call: `ASTNode* callee; ASTNode** args; int argCount;`
  - Get: `ASTNode* object; char* name;`
  - Binary: `Token op; ASTNode* left; ASTNode* right;`
  - Assignment: `ASTNode* target; ASTNode* value;`
  - Return: `ASTNode* value;`
  - Include: `char* filename;`

Creation helpers (signatures in `include/ast.h`):

- `ASTNode* createProgramNode(ASTNode** statements, int count);`
- `ASTNode* createFunctionNode(char* name, ASTNode** params, int paramCount, ASTNode* returnType, ASTNode* body);`
- `ASTNode* createVarNode(char* name, ASTNode* type, ASTNode* initializer);`
- `ASTNode* createLiteralNode(Token token);`
- `ASTNode* createVarRefNode(char* name);`
- `ASTNode* createBinaryNode(Token op, ASTNode* left, ASTNode* right);`
- `ASTNode* createAssignmentNode(ASTNode* target, ASTNode* value);`
- `ASTNode* createReturnNode(ASTNode* value);`
- `ASTNode* createIncludeNode(char* filename);`
- `ASTNode* createCallNode(ASTNode* callee, ASTNode** args, int argCount);`
- `ASTNode* createGetNode(ASTNode* object, char* name);`

Utility functions:

- `void freeAST(ASTNode* node);` — free node tree memory
- `void printAST(ASTNode* node, int depth);` — pretty-print AST for debugging

## Lexer API (include/lexer.h)

Struct: `Lexer`:

- `const char* start` — start pointer for current token
- `const char* current` — current scanning position
- `int line` — current line number

Functions:

- `void initLexer(Lexer* lexer, const char* source);` — initialize lexer with source buffer
- `Token scanToken(Lexer* lexer);` — return next `Token`

Note: tokens are described in `include/tokens.h`.

## Parser API (include/parser.h)

- `ASTNode* parse(const char* source);` — parse source into an `ASTNode*` representing the program. Returns `NULL` on parse failure.

## Semantic API (include/semantic.h)

Types:

- `SymbolType` enum: `SYM_VARIABLE`, `SYM_FUNCTION`, `SYM_PARAMETER`, `SYM_CLASS`.
- `Symbol` structure: holds `name`, `type` (SymbolType), `ASTNode* typeNode`, `scopeDepth`, `definedLine`, `next`.
- `SymbolTable` structure: hash buckets, capacity, count, current `scopeDepth`.
- `TypeInfo` structure: `char* name`, `int size`, flags and base type pointer.

Functions:

- `SymbolTable* createSymbolTable();`
- `void freeSymbolTable(SymbolTable* table);`
- `int enterScope(SymbolTable* table);` — push new scope
- `int exitScope(SymbolTable* table);` — pop scope
- `int defineSymbol(SymbolTable* table, const char* name, SymbolType type, ASTNode* typeNode, int line);`
- `Symbol* resolveSymbol(SymbolTable* table, const char* name);`
- `int typeCheck(ASTNode* node, SymbolTable* symbols);` — run semantic analysis; returns non-zero for success
- `TypeInfo* getTypeInfo(ASTNode* node, SymbolTable* symbols);` — get resolved type information
- `int areTypesCompatible(TypeInfo* t1, TypeInfo* t2);`
- `void printSymbolTable(SymbolTable* table);` — debugging helper

## Notes for contributors

- Use the create/free helpers when manipulating AST nodes to ensure memory consistency.
- `typeCheck` expects a fully constructed AST from `parse()` and a fresh `SymbolTable` created with `createSymbolTable()`.
- When extending node kinds, update `NodeType`, `ASTNode` union, creation helpers, parser, semantic checks and codegen.