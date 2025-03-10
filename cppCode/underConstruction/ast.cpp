#include "ast.h"

ASTNode::ASTNode(const std::string &type, const std::string &value, const std::vector<ASTNode *> &children)
    : type(type), value(value), children(children) {}

ASTNode::~ASTNode()
{
    for (ASTNode *child : children)
    {
        delete child;
    }
}