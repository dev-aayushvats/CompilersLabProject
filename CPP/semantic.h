#ifndef SEMANTIC_H
#define SEMANTIC_H

#include <unordered_map>
#include <stdexcept>
#include "parser.h" // For ASTNode

void semanticAnalysis(ASTNode* ast, std::unordered_map<std::string, std::string>& symbolTable) {
    if (ast->type == "DECLARATION") {
        symbolTable[ast->value] = "int";
    } else if (ast->type == "ASSIGNMENT" || ast->type == "IDENTIFIER") {
        if (symbolTable.find(ast->value) == symbolTable.end()) {
            throw std::runtime_error("Undefined variable: " + ast->value);
        }
    }
    for (ASTNode* child : ast->children) {
        semanticAnalysis(child, symbolTable);
    }
}

#endif