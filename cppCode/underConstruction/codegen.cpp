#include "codegen.h"
#include <iostream>

CodeGenerator::CodeGenerator() {}

std::string CodeGenerator::generate(ASTNode *ast)
{
    generateNode(ast);
    std::string jsCode = "";
    for (const std::string &line : js)
    {
        jsCode += line + "\n";
    }
    std::cout << "Output from Code Generation stage:\n"
              << jsCode;
    return jsCode;
}

void CodeGenerator::generateNode(ASTNode *ast)
{
    if (ast->type == "PROGRAM" || ast->type == "BLOCK")
    {
        for (ASTNode *child : ast->children)
            generateNode(child);
    }
    else if (ast->type == "DECLARATION")
    {
        js.push_back("let " + ast->value + " = " + generateExpression(ast->children[0]) + ";");
    }
    else if (ast->type == "ASSIGNMENT")
    {
        js.push_back(ast->value + " = " + generateExpression(ast->children[0]) + ";");
    }
    else if (ast->type == "FOR")
    {
        ASTNode *init = ast->children[0];
        ASTNode *cond = ast->children[1];
        ASTNode *update = ast->children[2];
        ASTNode *body = ast->children[3];
        std::string initStr = (init->type == "DECLARATION" ? "let " + init->value + " = " + generateExpression(init->children[0]) : generateExpression(init));
        std::string updateExpr = (update->type == "ASSIGNMENT" ? update->value + " = " + generateExpression(update->children[0]) : generateExpression(update));
        js.push_back("for (" + initStr + "; " + generateExpression(cond) + "; " + updateExpr + ") {");
        generateNode(body);
        js.push_back("}");
    }
    else if (ast->type == "WHILE")
    {
        js.push_back("while (" + generateExpression(ast->children[0]) + ") {");
        generateNode(ast->children[1]);
        js.push_back("}");
    }
    else if (ast->type == "DO_WHILE")
    {
        js.push_back("do {");
        generateNode(ast->children[0]);
        js.push_back("} while (" + generateExpression(ast->children[1]) + ");");
    }
    else if (ast->type == "IF")
    {
        js.push_back("if (" + generateExpression(ast->children[0]) + ") {");
        generateNode(ast->children[1]);
        if (ast->children.size() > 2)
        {
            js.push_back("} else {");
            generateNode(ast->children[2]);
        }
        js.push_back("}");
    }
    else if (ast->type == "SWITCH")
    {
        js.push_back("switch (" + generateExpression(ast->children[0]) + ") {");
        for (size_t i = 1; i < ast->children.size(); i++)
        {
            ASTNode *caseNode = ast->children[i];
            if (caseNode->type == "CASE")
            {
                js.push_back("case " + generateExpression(caseNode->value) + ":");
                for (ASTNode *stmt : caseNode->children)
                    generateNode(stmt);
            }
            else if (caseNode->type == "DEFAULT")
            {
                js.push_back("default:");
                for (ASTNode *stmt : caseNode->children)
                    generateNode(stmt);
            }
        }
        js.push_back("}");
    }
    else if (ast->type == "BREAK")
    {
        js.push_back("break;");
    }
}

std::string CodeGenerator::generateExpression(ASTNode *ast)
{
    if (ast->type == "NUMBER" || ast->type == "IDENTIFIER")
        return ast->value;
    if (ast->type == "BINOP")
    {
        std::string left = generateExpression(ast->children[0]);
        std::string right = generateExpression(ast->children[1]);
        return "(" + left + " " + ast->value + " " + right + ")";
    }
    if (ast->type == "COMPARE")
    {
        std::string left = generateExpression(ast->children[0]);
        std::string right = generateExpression(ast->children[1]);
        return left + " " + ast->value + " " + right;
    }
    return "";
}