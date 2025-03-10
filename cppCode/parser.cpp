#include "parser.h"
#include <stdexcept>
#include <iostream>

Parser::Parser(const std::vector<Token> &tokens) : tokens(tokens), pos(0) {}

void Parser::consume(const std::string &type)
{
    if (pos < tokens.size() && tokens[pos].type == type)
        pos++;
    else
    {
        std::string got = (pos < tokens.size()) ? tokens[pos].value : "EOF";
        throw std::runtime_error("Expected " + type + ", got " + got);
    }
}

ASTNode *Parser::parse()
{
    std::vector<ASTNode *> statements;
    while (pos < tokens.size())
    {
        statements.push_back(parseStatement());
    }
    ASTNode *ast = new ASTNode("PROGRAM", "", statements);
    std::cout << "Output from Parser stage: (AST structure not printed, imagine it here)" << std::endl;
    return ast;
}

ASTNode *Parser::parseStatement()
{
    const Token &token = tokens[pos];
    if (token.type == "INT")
        return parseDeclaration();
    if (token.type == "IDENTIFIER")
        return parseAssignment();
    throw std::runtime_error("Unexpected token: " + token.value);
}

ASTNode *Parser::parseDeclaration()
{
    pos++; // Skip 'int'
    std::string varName = tokens[pos++].value;
    consume("EQUALS");
    ASTNode *expr = parseExpression();
    consume("SEMICOLON");
    return new ASTNode("DECLARATION", varName, {expr});
}

ASTNode *Parser::parseAssignment()
{
    std::string varName = tokens[pos++].value;
    consume("EQUALS");
    ASTNode *expr = parseExpression();
    consume("SEMICOLON");
    return new ASTNode("ASSIGNMENT", varName, {expr});
}

ASTNode *Parser::parseExpression()
{
    ASTNode *node = nullptr;

    if (pos < tokens.size())
    {
        const Token &token = tokens[pos];
        if (token.type == "NUMBER")
        {
            node = new ASTNode("NUMBER", token.value);
            pos++;
        }
        else if (token.type == "IDENTIFIER")
        {
            node = new ASTNode("IDENTIFIER", token.value);
            pos++;
        }
        else
        {
            throw std::runtime_error("Unexpected token in expression: " + token.value);
        }
    }

    // Handle binary operations (e.g., 3 + 4)
    while (pos < tokens.size() && (tokens[pos].type == "PLUS" || tokens[pos].type == "MINUS" ||
                                   tokens[pos].type == "MULT" || tokens[pos].type == "DIV"))
    {
        std::string op = tokens[pos++].value;
        ASTNode *right = nullptr;
        if (pos < tokens.size())
        {
            const Token &nextToken = tokens[pos];
            if (nextToken.type == "NUMBER")
            {
                right = new ASTNode("NUMBER", nextToken.value);
                pos++;
            }
            else if (nextToken.type == "IDENTIFIER")
            {
                right = new ASTNode("IDENTIFIER", nextToken.value);
                pos++;
            }
            else
            {
                throw std::runtime_error("Expected number or identifier after operator, got: " + nextToken.value);
            }
        }
        node = new ASTNode("BINOP", op, {node, right});
    }

    return node;
}