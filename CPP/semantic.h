#ifndef SEMANTIC_H
#define SEMANTIC_H

#include <unordered_map>
#include <stdexcept>
#include "parser.h" // For ASTNode

void semanticAnalysis(ASTNode* ast, std::unordered_map<std::string, std::string>& symbolTable) {
    if (ast->type == "DECLARATION_INT") {
        symbolTable[ast->value] = "int";
        ASTNode* expr = ast->children[0];
        if (expr->type == "CHAR" || expr->type == "STRING") {
            throw std::runtime_error("Type mismatch: Cannot assign " + expr->type + " to int variable " + ast->value);
        } else if (expr->type == "IDENTIFIER") {
            if (symbolTable.find(expr->value) == symbolTable.end()) {
                throw std::runtime_error("Undefined variable: " + expr->value);
            }
            if (symbolTable[expr->value] != "int") {
                throw std::runtime_error("Type mismatch: " + expr->value + " is not an int");
            }
        }
    } else if (ast->type == "DECLARATION_CHAR_TYPE") {
        symbolTable[ast->value] = "char";
        ASTNode* expr = ast->children[0];
        if (expr->type != "CHAR") {
            throw std::runtime_error("Type mismatch: Cannot assign " + expr->type + " to char variable " + ast->value);
        }
    } else if (ast->type == "ASSIGNMENT") {
        if (symbolTable.find(ast->value) == symbolTable.end()) {
            throw std::runtime_error("Undefined variable: " + ast->value);
        }
        std::string varType = symbolTable[ast->value];
        ASTNode* expr = ast->children[0];
        if (varType == "int" && (expr->type == "CHAR" || expr->type == "STRING")) {
            throw std::runtime_error("Type mismatch: Cannot assign " + expr->type + " to int variable " + ast->value);
        } else if (varType == "char" && expr->type != "CHAR") {
            throw std::runtime_error("Type mismatch: Cannot assign " + expr->type + " to char variable " + ast->value);
        } else if (expr->type == "IDENTIFIER") {
            if (symbolTable.find(expr->value) == symbolTable.end()) {
                throw std::runtime_error("Undefined variable: " + expr->value);
            }
            if (symbolTable[expr->value] != varType) {
                throw std::runtime_error("Type mismatch: " + expr->value + " is not a " + varType);
            }
        }
    } else if (ast->type == "IDENTIFIER") {
        if (symbolTable.find(ast->value) == symbolTable.end()) {
            throw std::runtime_error("Undefined variable: " + ast->value);
        }
    }

    for (ASTNode* child : ast->children) {
        semanticAnalysis(child, symbolTable);
    }
}

#endif