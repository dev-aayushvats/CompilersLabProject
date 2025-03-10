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
    std::cout << "Output from Parser stage: (AST structure not printed in C++, imagine JSON here)" << std::endl;
    return ast;
}

ASTNode *Parser::parseStatement()
{
    const Token &token = tokens[pos];
    if (token.type == "INT")
        return parseDeclaration();
    if (token.type == "IDENTIFIER")
        return parseAssignment();
    if (token.type == "FOR")
        return parseFor();
    if (token.type == "WHILE")
        return parseWhile();
    if (token.type == "DO")
        return parseDoWhile();
    if (token.type == "IF")
        return parseIf();
    if (token.type == "SWITCH")
        return parseSwitch();
    if (token.type == "BREAK")
    {
        pos++;
        consume("SEMICOLON");
        return new ASTNode("BREAK", "break");
    }
    if (token.type == "LBRACE")
        return parseBlock();
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
    ASTNode *node = parseTerm();
    while (pos < tokens.size() && (tokens[pos].type == "PLUS" || tokens[pos].type == "MINUS"))
    {
        std::string op = tokens[pos++].value;
        ASTNode *right = parseTerm();
        node = new ASTNode("BINOP", op, {node, right});
    }
    return node;
}

ASTNode *Parser::parseTerm()
{
    ASTNode *node = parseFactor();
    while (pos < tokens.size() && (tokens[pos].type == "MULT" || tokens[pos].type == "DIV" || tokens[pos].type == "MOD"))
    {
        std::string op = tokens[pos++].value;
        ASTNode *right = parseFactor();
        node = new ASTNode("BINOP", op, {node, right});
    }
    return node;
}

ASTNode *Parser::parseFactor()
{
    const Token &token = tokens[pos];
    if (token.type == "NUMBER")
    {
        pos++;
        return new ASTNode("NUMBER", token.value);
    }
    if (token.type == "IDENTIFIER")
    {
        pos++;
        if (pos < tokens.size() && tokens[pos].type == "EQUALS")
        {
            pos++;
            ASTNode *right = parseExpression();
            return new ASTNode("ASSIGNMENT", token.value, {right});
        }
        return new ASTNode("IDENTIFIER", token.value);
    }
    if (token.type == "LPAREN")
    {
        pos++;
        ASTNode *expr = parseExpression();
        consume("RPAREN");
        return expr;
    }
    throw std::runtime_error("Unexpected token in expression: " + token.value);
}

ASTNode *Parser::parseComparison()
{
    ASTNode *left = parseExpression();
    if (pos < tokens.size())
    {
        const Token &op = tokens[pos];
        if (op.type == "LT" || op.type == "GT" || op.type == "LTE" || op.type == "GTE" || op.type == "EQ" || op.type == "NEQ")
        {
            pos++;
            ASTNode *right = parseExpression();
            return new ASTNode("COMPARE", op.value, {left, right});
        }
    }
    return left;
}

ASTNode *Parser::parseFor()
{
    pos++; // Skip 'for'
    consume("LPAREN");
    ASTNode *init = parseStatement();
    ASTNode *condition = parseComparison();
    consume("SEMICOLON");
    ASTNode *update = parseFactor();
    consume("RPAREN");
    ASTNode *body = parseStatement();
    return new ASTNode("FOR", "", {init, condition, update, body});
}

ASTNode *Parser::parseWhile()
{
    pos++; // Skip 'while'
    consume("LPAREN");
    ASTNode *condition = parseComparison();
    consume("RPAREN");
    ASTNode *body = parseStatement();
    return new ASTNode("WHILE", "", {condition, body});
}

ASTNode *Parser::parseDoWhile()
{
    pos++; // Skip 'do'
    ASTNode *body = parseStatement();
    consume("WHILE");
    consume("LPAREN");
    ASTNode *condition = parseComparison();
    consume("RPAREN");
    consume("SEMICOLON");
    return new ASTNode("DO_WHILE", "", {body, condition});
}

ASTNode *Parser::parseIf()
{
    pos++; // Skip 'if'
    consume("LPAREN");
    ASTNode *condition = parseComparison();
    consume("RPAREN");
    ASTNode *thenBranch = parseStatement();
    ASTNode *elseBranch = nullptr;
    if (pos < tokens.size() && tokens[pos].type == "ELSE")
    {
        pos++;
        elseBranch = parseStatement();
    }
    std::vector<ASTNode *> children = {condition, thenBranch};
    if (elseBranch)
        children.push_back(elseBranch);
    return new ASTNode("IF", "", children);
}

ASTNode *Parser::parseSwitch()
{
    pos++; // Skip 'switch'
    consume("LPAREN");
    ASTNode *expr = parseExpression();
    consume("RPAREN");
    consume("LBRACE");
    std::vector<ASTNode *> cases;
    while (pos < tokens.size() && tokens[pos].type != "RBRACE")
    {
        if (tokens[pos].type == "CASE")
        {
            pos++;
            ASTNode *value = parseExpression();
            consume("COLON");
            std::vector<ASTNode *> body;
            while (pos < tokens.size() && tokens[pos].type != "CASE" && tokens[pos].type != "DEFAULT" && tokens[pos].type != "RBRACE")
            {
                body.push_back(parseStatement());
            }
            cases.push_back(new ASTNode("CASE", "", {value}));
            cases.back()->children.insert(cases.back()->children.end(), body.begin(), body.end());
        }
        else if (tokens[pos].type == "DEFAULT")
        {
            pos++;
            consume("COLON");
            std::vector<ASTNode *> body;
            while (pos < tokens.size() && tokens[pos].type != "CASE" && tokens[pos].type != "DEFAULT" && tokens[pos].type != "RBRACE")
            {
                body.push_back(parseStatement());
            }
            cases.push_back(new ASTNode("DEFAULT", "", body));
        }
    }
    consume("RBRACE");
    return new ASTNode("SWITCH", expr->value, cases);
}

ASTNode *Parser::parseBlock()
{
    consume("LBRACE");
    std::vector<ASTNode *> statements;
    while (pos < tokens.size() && tokens[pos].type != "RBRACE")
    {
        statements.push_back(parseStatement());
    }
    consume("RBRACE");
    return new ASTNode("BLOCK", "", statements);
}