#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>

// Token class
class Token {
public:
    std::string type;
    std::string value;

    Token(const std::string& t, const std::string& v) : type(t), value(v) {}
};

// Lexer class
class Lexer {
private:
    std::string input;
    size_t pos;

    bool isAlpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
    bool isDigit(char c) { return c >= '0' && c <= '9'; }
    bool isWhitespace(char c) { return c == ' ' || c == '\t'; }

    // Skip comments and empty lines
    void skipCommentsAndEmptyLines() {
        while (pos < input.length()) {
            char current = input[pos];

            if (isWhitespace(current)) {
                pos++;
                continue;
            }

            if (current == '/' && pos + 1 < input.length() && input[pos + 1] == '/') {
                while (pos < input.length() && input[pos] != '\n') {
                    pos++;
                }
                pos++; // Skip the newline
                continue;
            }

            if (current == '/' && pos + 1 < input.length() && input[pos + 1] == '*') {
                pos += 2;
                while (pos + 1 < input.length() && !(input[pos] == '*' && input[pos + 1] == '/')) {
                    pos++;
                }
                pos += 2; // Skip the closing */
                continue;
            }

            if (current == '\n') {
                pos++;
                continue;
            }

            break;
        }
    }

public:
    Lexer(const std::string& source) : input(source), pos(0) {}

    std::vector<Token> tokenize() {
        std::vector<Token> tokens;

        while (pos < input.length()) {
            skipCommentsAndEmptyLines();

            if (pos >= input.length()) break;

            char current = input[pos];

            // Character literal (e.g., 'a')
            if (current == '\'') {
                std::string value;
                pos++; // Skip opening quote
                if (pos >= input.length()) {
                    throw std::runtime_error("Unterminated character literal");
                }
                value += input[pos++]; // Take one character
                if (pos >= input.length() || input[pos] != '\'') {
                    throw std::runtime_error("Expected closing single quote for character literal");
                }
                pos++; // Skip closing quote
                tokens.push_back(Token("CHAR", value));
                continue;
            }

            // String literal (e.g., "hello")
            if (current == '"') {
                std::string value;
                pos++; // Skip opening quote
                while (pos < input.length() && input[pos] != '"') {
                    value += input[pos++];
                }
                if (pos >= input.length()) {
                    throw std::runtime_error("Unterminated string literal");
                }
                pos++; // Skip closing quote
                tokens.push_back(Token("STRING", value));
                continue;
            }

            if (isAlpha(current)) {
                std::string value;
                while (pos < input.length() && (isAlpha(input[pos]) || isDigit(input[pos]))) {
                    value += input[pos++];
                }
                if (value == "int" || value == "char") {
                    tokens.push_back(Token(value == "int" ? "INT" : "CHAR_TYPE", value));
                } else {
                    tokens.push_back(Token("IDENTIFIER", value));
                }
                continue;
            }

            if (isDigit(current)) {
                std::string value;
                while (pos < input.length() && isDigit(input[pos])) {
                    value += input[pos++];
                }
                tokens.push_back(Token("NUMBER", value));
                continue;
            }

            switch (current) {
                case '+': tokens.push_back(Token("PLUS", "+")); pos++; break;
                case '-': tokens.push_back(Token("MINUS", "-")); pos++; break;
                case '*': tokens.push_back(Token("MULT", "*")); pos++; break;
                case '/': tokens.push_back(Token("DIV", "/")); pos++; break;
                case '=': tokens.push_back(Token("EQUALS", "=")); pos++; break;
                case ';': tokens.push_back(Token("SEMICOLON", ";")); pos++; break;
                default: pos++;
            }
        }

        return tokens;
    }
};

#endif