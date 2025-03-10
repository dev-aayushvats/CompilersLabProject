#ifndef SEMANTIC_H
#define SEMANTIC_H

#include <map>
#include "ast.h"

std::map<std::string, std::string> semanticAnalysis(ASTNode *ast, std::map<std::string, std::string> symbolTable = {});

#endif