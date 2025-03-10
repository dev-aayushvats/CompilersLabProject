#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include "token.h"
#include "ast.h"

class Parser
{
private:
    std::vector<Token> tokens;
    size_t pos;

    void consume(const std::string &type);
    ASTNode *parseStatement();
    ASTNode *parseDeclaration();
    ASTNode *parseAssignment();
    ASTNode *parseExpression();
    ASTNode *parseTerm();
    ASTNode *parseFactor();
    ASTNode *parseComparison();
    ASTNode *parseFor();
    ASTNode *parseWhile();
    ASTNode *parseDoWhile();
    ASTNode *parseIf();
    ASTNode *parseSwitch();
    ASTNode *parseBlock();

public:
    Parser(const std::vector<Token> &tokens);
    ASTNode *parse();
};

#endif