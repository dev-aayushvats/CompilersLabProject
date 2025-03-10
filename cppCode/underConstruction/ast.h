#ifndef AST_H
#define AST_H

#include <string>
#include <vector>

class ASTNode
{
public:
    std::string type;
    std::string value;
    std::vector<ASTNode *> children;

    ASTNode(const std::string &type, const std::string &value, const std::vector<ASTNode *> &children = {});
    ~ASTNode();
};

#endif