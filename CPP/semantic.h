#ifndef SEMANTIC_H
#define SEMANTIC_H

#include <unordered_map>
#include <stdexcept>
#include <vector>
#include <string>
#include "parser.h" // For ASTNode

class SymbolTable {
private:
    std::vector<std::unordered_map<std::string, std::string>> scopes;

public:
    SymbolTable() {
        // Initialize with global scope
        enterScope();
    }

    void enterScope() {
        scopes.push_back(std::unordered_map<std::string, std::string>());
    }

    void exitScope() {
        if (scopes.size() > 1) { // Always keep at least global scope
            scopes.pop_back();
        }
    }

    void define(const std::string& name, const std::string& type) {
        // Add to current scope
        scopes.back()[name] = type;
    }

    bool isDefined(const std::string& name) const {
        // Check all scopes from local to global
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
            if (it->find(name) != it->end()) {
                return true;
            }
        }
        return false;
    }

    std::string getType(const std::string& name) const {
        // Get type from innermost scope where name is defined
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
            auto found = it->find(name);
            if (found != it->end()) {
                return found->second;
            }
        }
        throw std::runtime_error("Undefined variable: " + name);
    }

    void getAllSymbols(std::unordered_map<std::string, std::string>& outTable) const {
        // Copy all symbols from all scopes to outTable
        for (const auto& scope : scopes) {
            for (const auto& [name, type] : scope) {
                outTable[name] = type;
            }
        }
    }
};

void analyzeNode(ASTNode* ast, SymbolTable& symbolTable);

// Forward declarations for analyzers
void analyzeBlock(ASTNode* node, SymbolTable& symbolTable);
void analyzeFunction(ASTNode* node, SymbolTable& symbolTable);
void analyzeIfStatement(ASTNode* node, SymbolTable& symbolTable);
void analyzeWhileLoop(ASTNode* node, SymbolTable& symbolTable);
void analyzeForLoop(ASTNode* node, SymbolTable& symbolTable);
void analyzeExpression(ASTNode* node, SymbolTable& symbolTable);

// Main semantic analysis function
void semanticAnalysis(ASTNode* ast, std::unordered_map<std::string, std::string>& outSymbolTable) {
    SymbolTable symbolTable;
    analyzeNode(ast, symbolTable);
    symbolTable.getAllSymbols(outSymbolTable);
}

// Recursive analysis function
void analyzeNode(ASTNode* ast, SymbolTable& symbolTable) {
    if (!ast) return;

    if (ast->type == "PROGRAM") {
        // Process all program children
        for (ASTNode* child : ast->children) {
            analyzeNode(child, symbolTable);
        }
    }
    else if (ast->type == "INCLUDE") {
        // Nothing to analyze for includes
    }
    else if (ast->type == "FUNCTION") {
        analyzeFunction(ast, symbolTable);
    }
    else if (ast->type == "BLOCK") {
        analyzeBlock(ast, symbolTable);
    }
    else if (ast->type == "IF") {
        analyzeIfStatement(ast, symbolTable);
    }
    else if (ast->type == "WHILE") {
        analyzeWhileLoop(ast, symbolTable);
    }
    else if (ast->type == "FOR") {
        analyzeForLoop(ast, symbolTable);
    }
    else if (ast->type == "DECLARATION_INT") {
        symbolTable.define(ast->value, "int");
        if (!ast->children.empty()) { // Check if initialized
            ASTNode* expr = ast->children[0];
            analyzeExpression(expr, symbolTable);
            
            if (expr->type == "CHAR" || expr->type == "STRING") {
                throw std::runtime_error("Type mismatch: Cannot assign " + expr->type + " to int variable " + ast->value);
            } else if (expr->type == "IDENTIFIER") {
                std::string exprType = symbolTable.getType(expr->value);
                if (exprType != "int") {
                    throw std::runtime_error("Type mismatch: " + expr->value + " is not an int");
                }
            }
        }
    }
    else if (ast->type == "DECLARATION_CHAR_TYPE") {
        symbolTable.define(ast->value, "char");
        if (!ast->children.empty()) { // Check if initialized
            ASTNode* expr = ast->children[0];
            analyzeExpression(expr, symbolTable);
            
            if (expr->type != "CHAR") {
                throw std::runtime_error("Type mismatch: Cannot assign " + expr->type + " to char variable " + ast->value);
            }
        }
    }
    else if (ast->type == "ASSIGNMENT") {
        if (!symbolTable.isDefined(ast->value)) {
            throw std::runtime_error("Undefined variable: " + ast->value);
        }
        
        std::string varType = symbolTable.getType(ast->value);
        ASTNode* expr = ast->children[0];
        analyzeExpression(expr, symbolTable);
        
        if (varType == "int") {
            if (expr->type == "CHAR" || expr->type == "STRING") {
                throw std::runtime_error("Type mismatch: Cannot assign " + expr->type + " to int variable " + ast->value);
            } else if (expr->type == "IDENTIFIER") {
                std::string exprType = symbolTable.getType(expr->value);
                if (exprType != "int") {
                    throw std::runtime_error("Type mismatch: " + expr->value + " is not an int");
                }
            }
        } else if (varType == "char") {
            if (expr->type == "IDENTIFIER") {
                std::string exprType = symbolTable.getType(expr->value);
                if (exprType != "char") {
                    throw std::runtime_error("Type mismatch: " + expr->value + " is not a char");
                }
            } else if (expr->type != "CHAR") {
                throw std::runtime_error("Type mismatch: Cannot assign " + expr->type + " to char variable " + ast->value);
            }
        }
    }
    else if (ast->type == "IDENTIFIER") {
        if (!symbolTable.isDefined(ast->value)) {
            throw std::runtime_error("Undefined variable: " + ast->value);
        }
    }
    else if (ast->type == "RETURN") {
        if (!ast->children.empty()) {
            analyzeExpression(ast->children[0], symbolTable);
            // We could add return type checking here
        }
    }
    else {
        // Process any unknown node types
        for (ASTNode* child : ast->children) {
            analyzeNode(child, symbolTable);
        }
    }
}

void analyzeBlock(ASTNode* node, SymbolTable& symbolTable) {
    symbolTable.enterScope();
    
    for (ASTNode* child : node->children) {
        analyzeNode(child, symbolTable);
    }
    
    symbolTable.exitScope();
}

void analyzeFunction(ASTNode* node, SymbolTable& symbolTable) {
    // Define function in symbol table
    symbolTable.define(node->value, node->children[0]->value); // Return type
    
    // Analyze function body (should be a block)
    if (node->children.size() > 1) {
        analyzeNode(node->children[1], symbolTable);
    }
}

void analyzeIfStatement(ASTNode* node, SymbolTable& symbolTable) {
    // Analyze condition
    analyzeExpression(node->children[0], symbolTable);
    
    // Analyze then branch
    analyzeNode(node->children[1], symbolTable);
    
    // Analyze optional else branch
    if (node->children.size() > 2) {
        analyzeNode(node->children[2], symbolTable);
    }
}

void analyzeWhileLoop(ASTNode* node, SymbolTable& symbolTable) {
    // Analyze condition
    analyzeExpression(node->children[0], symbolTable);
    
    // Analyze body
    analyzeNode(node->children[1], symbolTable);
}

void analyzeForLoop(ASTNode* node, SymbolTable& symbolTable) {
    symbolTable.enterScope();
    
    // Analyze initialization
    analyzeNode(node->children[0], symbolTable);
    
    // Analyze condition
    analyzeExpression(node->children[1], symbolTable);
    
    // Analyze update
    analyzeExpression(node->children[2], symbolTable);
    
    // Analyze body
    analyzeNode(node->children[3], symbolTable);
    
    symbolTable.exitScope();
}

void analyzeExpression(ASTNode* node, SymbolTable& symbolTable) {
    if (!node) return;
    
    if (node->type == "IDENTIFIER") {
        if (!symbolTable.isDefined(node->value)) {
            throw std::runtime_error("Undefined variable: " + node->value);
        }
    }
    else if (node->type == "BINOP" || node->type == "LOGICAL_OP" || 
             node->type == "COMPARISON_OP" || node->type == "UNARY_OP") {
        for (ASTNode* child : node->children) {
            analyzeExpression(child, symbolTable);
        }
    }
    
    // For other expression types (literals), no analysis needed
}

#endif