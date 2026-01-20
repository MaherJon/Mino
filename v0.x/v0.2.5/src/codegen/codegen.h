#ifndef MINO_CODEGEN_H
#define MINO_CODEGEN_H

#include <ast.h>

// Generate an executable; returns 0 on success
int codegen_generateExecutable(ASTNode* ast, const char* outPath);

#endif
