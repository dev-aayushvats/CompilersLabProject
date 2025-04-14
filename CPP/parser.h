#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <cstddef> // For size_t
#include "lexer.h"

// Forward declaration
class Token;

// ASTNode class
class ASTNode {
public:
    std::string type;
    std::string value;
    std::vector<ASTNode*> children;

    ASTNode(const std::string& t, const std::string& v = "") : type(t), value(v) {}
    ~ASTNode() {
        for (auto& child : children) {
            delete child;
        }
    }

    void addChild(ASTNode* child) {
        children.push_back(child);
    }

    void print(int level = 0, bool isLast = true, std::string prefix = "") const {
        if (level == 0) {
            std::cout << "===== Abstract Syntax Tree (AST) =====\n";
        }

        std::string indent = level == 0 ? "" : prefix + (isLast ? "└── " : "├── ");
        std::cout << indent << type;
        if (!value.empty()) {
            std::cout << " (" << value << ")";
        }
        std::cout << "\n";

        std::string newPrefix = level == 0 ? "" : prefix + (isLast ? "    " : "│   ");
        for (size_t i = 0; i < children.size(); ++i) {
            bool childIsLast = (i == children.size() - 1);
            children[i]->print(level + 1, childIsLast, newPrefix);
        }

        if (level == 0) {
            std::cout << "======================================\n";
        }
    }
};

// Parser class
class Parser {
private:
    std::vector<Token> tokens;
    size_t pos;

    Token peek(size_t offset = 0) {
        if (pos + offset >= tokens.size()) {
            return Token("EOF", "");
        }
        return tokens[pos + offset];
    }

    bool match(const std::string& type) {
        if (pos >= tokens.size() || tokens[pos].type != type) {
            return false;
        }
        pos++;
        return true;
    }

    void consume(const std::string& type) {
        if (pos >= tokens.size() || tokens[pos].type != type) {
            throw std::runtime_error("Expected " + type + ", got " + 
                (pos < tokens.size() ? tokens[pos].type + " (" + tokens[pos].value + ")" : "EOF"));
        }
        pos++;
    }

    // Parse includes and directives
    ASTNode* parseInclude() {
        consume("DIRECTIVE"); // #include
        std::string headerName = tokens[pos].value;
        consume("HEADER");    // <iostream>, etc.
        
        ASTNode* includeNode = new ASTNode("INCLUDE", headerName);
        return includeNode;
    }

    // Parse function definition
    ASTNode* parseFunction() {
        // Return type
        std::string returnType = tokens[pos].value;
        consume("INT"); // Currently only supporting int return type
        
        // Function name
        std::string functionName = tokens[pos].value;
        consume("IDENTIFIER");
        
        // Parameters (currently just empty)
        consume("LPAREN");
        consume("RPAREN");
        
        // Function body
        ASTNode* body = parseBlock();
        
        ASTNode* functionNode = new ASTNode("FUNCTION", functionName);
        functionNode->addChild(new ASTNode("RETURN_TYPE", returnType));
        functionNode->addChild(body);
        
        return functionNode;
    }

    // Parse a block of statements
    ASTNode* parseBlock() {
        consume("LBRACE");
        
        ASTNode* blockNode = new ASTNode("BLOCK");
        
        while (pos < tokens.size() && tokens[pos].type != "RBRACE") {
            blockNode->addChild(parseStatement());
        }
        
        consume("RBRACE");
        return blockNode;
    }

    // Parse a statement
    ASTNode* parseStatement() {
        // Variable declaration
        if (tokens[pos].type == "INT" || tokens[pos].type == "CHAR_TYPE") {
            std::string varType = tokens[pos].type; // "INT" or "CHAR_TYPE"
            pos++; // Skip 'int' or 'char'
            std::string varName = tokens[pos].value;
            consume("IDENTIFIER");
            
            ASTNode* decl = new ASTNode("DECLARATION_" + varType, varName);
            
            if (match("EQUALS")) {
                ASTNode* expr = parseExpression();
                decl->addChild(expr);
            }
            
            consume("SEMICOLON");
            return decl;
        }
        // If statement
        else if (tokens[pos].type == "IF") {
            return parseIfStatement();
        }
        // While loop
        else if (tokens[pos].type == "WHILE") {
            return parseWhileLoop();
        }
        // For loop
        else if (tokens[pos].type == "FOR") {
            return parseForLoop();
        }
        // Return statement
        else if (tokens[pos].type == "RETURN") {
            pos++; // Skip 'return'
            ASTNode* returnNode = new ASTNode("RETURN");
            
            if (tokens[pos].type != "SEMICOLON") {
                returnNode->addChild(parseExpression());
            }
            
            consume("SEMICOLON");
            return returnNode;
        }
        // Assignment or expression statement
        else {
            ASTNode* expr = parseExpression();
            consume("SEMICOLON");
            return expr;
        }
    }

    // Parse if statement
    ASTNode* parseIfStatement() {
        consume("IF");
        consume("LPAREN");
        ASTNode* condition = parseExpression();
        consume("RPAREN");
        
        ASTNode* thenBranch = parseBlock();
        
        ASTNode* ifNode = new ASTNode("IF");
        ifNode->addChild(condition);
        ifNode->addChild(thenBranch);
        
        // Check for optional else
        if (pos < tokens.size() && tokens[pos].type == "ELSE") {
            consume("ELSE");
            
            // Handle else-if or else block
            if (tokens[pos].type == "IF") {
                ifNode->addChild(parseIfStatement());
            } else {
                ifNode->addChild(parseBlock());
            }
        }
        
        return ifNode;
    }

    // Parse while loop
    ASTNode* parseWhileLoop() {
        consume("WHILE");
        consume("LPAREN");
        ASTNode* condition = parseExpression();
        consume("RPAREN");
        
        ASTNode* body = parseBlock();
        
        ASTNode* whileNode = new ASTNode("WHILE");
        whileNode->addChild(condition);
        whileNode->addChild(body);
        
        return whileNode;
    }

    // Parse for loop
    ASTNode* parseForLoop() {
        consume("FOR");
        consume("LPAREN");
        
        // Initialization
        ASTNode* init = nullptr;
        if (tokens[pos].type == "INT") {
            init = parseStatement(); // Variable declaration with semicolon
        } else {
            init = parseExpression();
            consume("SEMICOLON");
        }
        
        // Condition
        ASTNode* condition = parseExpression();
        consume("SEMICOLON");
        
        // Update
        ASTNode* update = parseExpression();
        consume("RPAREN");
        
        // Body
        ASTNode* body = parseBlock();
        
        ASTNode* forNode = new ASTNode("FOR");
        forNode->addChild(init);
        forNode->addChild(condition);
        forNode->addChild(update);
        forNode->addChild(body);
        
        return forNode;
    }

    // Parse expressions
    ASTNode* parseExpression() {
        return parseAssignment();
    }
    
    ASTNode* parseAssignment() {
        if (tokens[pos].type == "IDENTIFIER" && pos + 1 < tokens.size() && tokens[pos + 1].type == "EQUALS") {
            std::string varName = tokens[pos].value;
            consume("IDENTIFIER");
            consume("EQUALS");
            
            ASTNode* expr = parseLogicalOr();
            ASTNode* assignNode = new ASTNode("ASSIGNMENT", varName);
            assignNode->addChild(expr);
            
            return assignNode;
        }
        
        return parseLogicalOr();
    }
    
    ASTNode* parseLogicalOr() {
        ASTNode* left = parseLogicalAnd();
        
        while (pos < tokens.size() && tokens[pos].type == "OR") {
            std::string op = tokens[pos].value;
            pos++;
            
            ASTNode* right = parseLogicalAnd();
            
            ASTNode* node = new ASTNode("LOGICAL_OP", op);
            node->addChild(left);
            node->addChild(right);
            
            left = node;
        }
        
        return left;
    }
    
    ASTNode* parseLogicalAnd() {
        ASTNode* left = parseEquality();
        
        while (pos < tokens.size() && tokens[pos].type == "AND") {
            std::string op = tokens[pos].value;
            pos++;
            
            ASTNode* right = parseEquality();
            
            ASTNode* node = new ASTNode("LOGICAL_OP", op);
            node->addChild(left);
            node->addChild(right);
            
            left = node;
        }
        
        return left;
    }
    
    ASTNode* parseEquality() {
        ASTNode* left = parseComparison();
        
        while (pos < tokens.size() && 
               (tokens[pos].type == "EQUALITY" || tokens[pos].type == "INEQUALITY")) {
            std::string op = tokens[pos].value;
            pos++;
            
            ASTNode* right = parseComparison();
            
            ASTNode* node = new ASTNode("COMPARISON_OP", op);
            node->addChild(left);
            node->addChild(right);
            
            left = node;
        }
        
        return left;
    }
    
    ASTNode* parseComparison() {
        ASTNode* left = parseAdditive();
        
        while (pos < tokens.size() && 
               (tokens[pos].type == "LESS" || tokens[pos].type == "LESS_EQUAL" || 
                tokens[pos].type == "GREATER" || tokens[pos].type == "GREATER_EQUAL")) {
            std::string op = tokens[pos].value;
            pos++;
            
            ASTNode* right = parseAdditive();
            
            ASTNode* node = new ASTNode("COMPARISON_OP", op);
            node->addChild(left);
            node->addChild(right);
            
            left = node;
        }
        
        return left;
    }
    
    ASTNode* parseAdditive() {
        ASTNode* left = parseTerm();
        
        while (pos < tokens.size() && 
               (tokens[pos].type == "PLUS" || tokens[pos].type == "MINUS")) {
            std::string op = tokens[pos].value;
            pos++;
            
            ASTNode* right = parseTerm();
            
            ASTNode* node = new ASTNode("BINOP", op);
            node->addChild(left);
            node->addChild(right);
            
            left = node;
        }
        
        return left;
    }
    
    ASTNode* parseTerm() {
        ASTNode* left = parseFactor();
        
        while (pos < tokens.size() && 
               (tokens[pos].type == "MULT" || tokens[pos].type == "DIV")) {
            std::string op = tokens[pos].value;
            pos++;
            
            ASTNode* right = parseFactor();
            
            ASTNode* node = new ASTNode("BINOP", op);
            node->addChild(left);
            node->addChild(right);
            
            left = node;
        }
        
        return left;
    }
    
    ASTNode* parseFactor() {
        if (pos >= tokens.size()) {
            throw std::runtime_error("Unexpected EOF in expression");
        }
        
        // Handle unary operators
        if (tokens[pos].type == "MINUS" || tokens[pos].type == "NOT") {
            std::string op = tokens[pos].value;
            pos++;
            
            ASTNode* operand = parseFactor();
            
            ASTNode* node = new ASTNode("UNARY_OP", op);
            node->addChild(operand);
            
            return node;
        }
        
        // Handle parentheses
        if (tokens[pos].type == "LPAREN") {
            pos++; // Skip '('
            ASTNode* expr = parseExpression();
            consume("RPAREN");
            return expr;
        }
        
        // Handle literals and identifiers
        Token token = tokens[pos++];
        
        if (token.type == "NUMBER") {
            return new ASTNode("NUMBER", token.value);
        } else if (token.type == "CHAR") {
            return new ASTNode("CHAR", token.value);
        } else if (token.type == "STRING") {
            return new ASTNode("STRING", token.value);
        } else if (token.type == "IDENTIFIER") {
            return new ASTNode("IDENTIFIER", token.value);
        }
        
        throw std::runtime_error("Unexpected token in expression: " + token.value);
    }

public:
    Parser(const std::vector<Token>& t) : tokens(t), pos(0) {}

    ASTNode* parse() {
        ASTNode* root = new ASTNode("PROGRAM");
        
        while (pos < tokens.size()) {
            if (tokens[pos].type == "DIRECTIVE") {
                // Parse #include directive
                root->addChild(parseInclude());
            } else if (tokens[pos].type == "INT" && 
                       pos + 1 < tokens.size() && tokens[pos + 1].type == "IDENTIFIER" &&
                       pos + 2 < tokens.size() && tokens[pos + 2].type == "LPAREN") {
                // Parse function definition (including main)
                root->addChild(parseFunction());
            } else if (tokens[pos].type == "INT" || tokens[pos].type == "CHAR_TYPE") {
                // Global variable declaration
                ASTNode* decl = parseStatement();
                root->addChild(decl);
            } else {
                // Skip unrecognized tokens
                pos++;
            }
        }
        
        return root;
    }
};

#endif