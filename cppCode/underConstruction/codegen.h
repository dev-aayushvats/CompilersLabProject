#ifndef CODEGEN_H
#define CODEGEN_H

#include <vector>
#include <string>
#include "ast.h"

class CodeGenerator
{
private:
    std::vector<std::string> js;

    void generateNode(ASTNode *ast);
    std::string generateExpression(ASTNode *ast);

public:
    CodeGenerator();
    std::string generate(ASTNode *ast);
};

#endif