#include "lexer.h"
#include <iostream>
#include <regex>

std::vector<Token> lexer(const std::string &input)
{
    std::vector<Token> tokens;
    size_t pos = 0;

    while (pos < input.length())
    {
        char char_at = input[pos];

        if (std::isspace(char_at))
        {
            pos++;
            continue;
        }

        if (std::isalpha(char_at))
        {
            std::string value;
            while (pos < input.length() && std::isalnum(input[pos]))
            {
                value += input[pos++];
            }
            const std::string keywords[] = {"int", "for", "while", "do", "if", "else", "switch", "case", "default", "break"};
            bool isKeyword = false;
            for (const std::string &kw : keywords)
            {
                if (value == kw)
                {
                    std::string upper_kw = kw;
                    for (char &c : upper_kw)
                        c = std::toupper(c);
                    tokens.push_back(Token(upper_kw, value));
                    isKeyword = true;
                    break;
                }
            }
            if (!isKeyword)
                tokens.push_back(Token("IDENTIFIER", value));
            continue;
        }

        if (std::isdigit(char_at))
        {
            std::string value;
            while (pos < input.length() && std::isdigit(input[pos]))
            {
                value += input[pos++];
            }
            tokens.push_back(Token("NUMBER", value));
            continue;
        }

        std::string single_chars = "+-*/%=<>(){};:,";
        const char *types[] = {"PLUS", "MINUS", "MULT", "DIV", "MOD", "EQUALS", "LT", "GT", "LPAREN", "RPAREN", "LBRACE", "RBRACE", "SEMICOLON", "COLON", "COMMA"};
        size_t idx = single_chars.find(char_at);
        if (idx != std::string::npos)
        {
            tokens.push_back(Token(types[idx], std::string(1, char_at)));
            pos++;
            continue;
        }

        if (char_at == '=' && pos + 1 < input.length() && input[pos + 1] == '=')
        {
            tokens.push_back(Token("EQ", "=="));
            pos += 2;
            continue;
        }
        if (char_at == '!' && pos + 1 < input.length() && input[pos + 1] == '=')
        {
            tokens.push_back(Token("NEQ", "!="));
            pos += 2;
            continue;
        }
        if (char_at == '<' && pos + 1 < input.length() && input[pos + 1] == '=')
        {
            tokens.push_back(Token("LTE", "<="));
            pos += 2;
            continue;
        }
        if (char_at == '>' && pos + 1 < input.length() && input[pos + 1] == '=')
        {
            tokens.push_back(Token("GTE", ">="));
            pos += 2;
            continue;
        }

        pos++;
    }

    std::cout << "Output from Lexer stage: ";
    for (const Token &t : tokens)
    {
        std::cout << "(" << t.type << ", " << t.value << ") ";
    }
    std::cout << std::endl;

    return tokens;
}