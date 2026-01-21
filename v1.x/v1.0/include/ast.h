// include/ast.h
#ifndef MINO_AST_H
#define MINO_AST_H

#include <tokens.h>

// AST node types
typedef enum {
    NODE_PROGRAM,
    NODE_FUNCTION_DECL,
    NODE_CLASS_DECL,
    NODE_VAR_DECL,
    NODE_EXPR_STMT,
    NODE_RETURN_STMT,
    NODE_IF_STMT,
    NODE_WHILE_STMT,
    NODE_BLOCK_STMT,
    NODE_BINARY_EXPR,
    NODE_UNARY_EXPR,
    NODE_CALL_EXPR,
    NODE_GET_EXPR,
    NODE_SET_EXPR,
    NODE_LITERAL,
    NODE_VARIABLE,
    NODE_ASSIGN,
    NODE_INCLUDE
} NodeType;

// Basic AST structure
typedef struct ASTNode ASTNode;

struct ASTNode {
    NodeType type;
    int line;
    
    union {
        // Program node
        struct {
            ASTNode** statements;
            int count;
        } program;
        
        struct {
            char* name;
            ASTNode** params;
            int paramCount;
            ASTNode* returnType;
            ASTNode* body;
        } function;
        
        struct {
            char* name;
            ASTNode* type;
            ASTNode* initializer;
        } variable;
        
        struct {
            Token token;
        } literal;
        
        struct {
            char* name;
        } varRef;

            struct {
                ASTNode* callee;
                ASTNode** args;
                int argCount;
            } call;

            struct {
                ASTNode* object;
                char* name;
            } get;
        
        struct {
            Token op;
            ASTNode* left;
            ASTNode* right;
        } binary;
        
        struct {
            ASTNode* target;
            ASTNode* value;
        } assignment;
        
        struct {
            ASTNode* value;
        } returnStmt;
        
        struct {
            char* filename;
        } include;
    };
};

// AST creation
ASTNode* createProgramNode(ASTNode** statements, int count);
ASTNode* createFunctionNode(char* name, ASTNode** params, int paramCount, 
                           ASTNode* returnType, ASTNode* body);
ASTNode* createVarNode(char* name, ASTNode* type, ASTNode* initializer);
ASTNode* createLiteralNode(Token token);
ASTNode* createVarRefNode(char* name);
ASTNode* createBinaryNode(Token op, ASTNode* left, ASTNode* right);
ASTNode* createAssignmentNode(ASTNode* target, ASTNode* value);
ASTNode* createReturnNode(ASTNode* value);
ASTNode* createIncludeNode(char* filename);
ASTNode* createCallNode(ASTNode* callee, ASTNode** args, int argCount);
ASTNode* createGetNode(ASTNode* object, char* name);

// AST free
void freeAST(ASTNode* node);

void printAST(ASTNode* node, int depth);

#endif