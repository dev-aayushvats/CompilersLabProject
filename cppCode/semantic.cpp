#include "semantic.h"
#include <iostream>

std::map<std::string, std::string> semanticAnalysis(ASTNode *ast, std::map<std::string, std::string> symbolTable)
{
    if (ast->type == "DECLARATION")
    {
        symbolTable[ast->value] = "int";
    }
    else if (ast->type == "ASSIGNMENT" || ast->type == "IDENTIFIER")
    {
        if (symbolTable.find(ast->value) == symbolTable.end())
        {
            throw std::runtime_error("Undefined variable: " + ast->value);
        }
    }
    for (ASTNode *child : ast->children)
    {
        symbolTable = semanticAnalysis(child, symbolTable);
    }
    std::cout << "Output from Semantic Analysis stage: (Symbol table not printed in C++, imagine JSON here)" << std::endl;
    return symbolTable;
}