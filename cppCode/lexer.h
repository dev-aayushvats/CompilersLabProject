#ifndef LEXER_H
#define LEXER_H

#include <vector>
#include "token.h"

std::vector<Token> lexer(const std::string &input);

#endif