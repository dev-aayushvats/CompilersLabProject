#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>
#include <stdexcept>
#include <iostream>

// ASTNode class
class ASTNode {
public:
    std::string type;
    std::string value;
    std::vector<ASTNode*> children;

    ASTNode(const std::string& t, const std::string& v = "") : type(t), value(v) {}
    ~ASTNode() {
        for (ASTNode* child : children) {
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

    void consume(const std::string& type) {
        if (pos >= tokens.size() || tokens[pos].type != type) {
            throw std::runtime_error("Expected " + type + ", got " + 
                (pos < tokens.size() ? tokens[pos].value : "EOF"));
        }
        pos++;
    }

    ASTNode* parseExpression() {
        ASTNode* left = parseTerm();
        while (pos < tokens.size() && (tokens[pos].type == "PLUS" || tokens[pos].type == "MINUS")) {
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
        while (pos < tokens.size() && (tokens[pos].type == "MULT" || tokens[pos].type == "DIV")) {
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
        Token token = tokens[pos++];
        if (token.type == "NUMBER") {
            return new ASTNode("NUMBER", token.value);
        } else if (token.type == "CHAR") {
            return new ASTNode("CHAR", token.value);
        } else if (token.type == "STRING") {
            return new ASTNode("STRING", token.value);
        } else if (token.type == "IDENTIFIER") {
            if (pos < tokens.size() && tokens[pos].type == "EQUALS") {
                pos++; // Skip '='
                ASTNode* expr = parseExpression();
                ASTNode* node = new ASTNode("ASSIGNMENT", token.value);
                node->addChild(expr);
                return node;
            }
            return new ASTNode("IDENTIFIER", token.value);
        }
        throw std::runtime_error("Unexpected token in expression: " + token.value);
    }

public:
    Parser(const std::vector<Token>& t) : tokens(t), pos(0) {}

    ASTNode* parse() {
        std::vector<ASTNode*> statements;
        while (pos < tokens.size()) {
            if (tokens[pos].type == "INT" || tokens[pos].type == "CHAR_TYPE") {
                std::string varType = tokens[pos].type; // "INT" or "CHAR_TYPE"
                pos++; // Skip 'int' or 'char'
                std::string varName = tokens[pos++].value;
                consume("EQUALS");
                ASTNode* expr = parseExpression();
                consume("SEMICOLON");
                ASTNode* decl = new ASTNode("DECLARATION_" + varType, varName);
                decl->addChild(expr);
                statements.push_back(decl);
            } else if (tokens[pos].type == "IDENTIFIER") {
                ASTNode* assign = parseFactor();
                consume("SEMICOLON");
                statements.push_back(assign);
            } else {
                throw std::runtime_error("Unexpected token: " + tokens[pos].value);
            }
        }
        ASTNode* root = new ASTNode("PROGRAM");
        for (ASTNode* stmt : statements) {
            root->addChild(stmt);
        }
        return root;
    }
};

#endif