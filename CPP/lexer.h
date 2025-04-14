#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include <unordered_map>

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
    std::unordered_map<std::string, std::string> keywords;

    bool isAlpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; }
    bool isDigit(char c) { return c >= '0' && c <= '9'; }
    bool isWhitespace(char c) { return c == ' ' || c == '\t'; }
    bool isAlphaNumeric(char c) { return isAlpha(c) || isDigit(c); }

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
    Lexer(const std::string& source) : input(source), pos(0) {
        // Initialize keywords map
        keywords["int"] = "INT";
        keywords["char"] = "CHAR_TYPE";
        keywords["if"] = "IF";
        keywords["else"] = "ELSE";
        keywords["for"] = "FOR";
        keywords["while"] = "WHILE";
        keywords["return"] = "RETURN";
        keywords["include"] = "INCLUDE";
        keywords["iostream"] = "IOSTREAM";
    }

    std::vector<Token> tokenize() {
        std::vector<Token> tokens;

        while (pos < input.length()) {
            skipCommentsAndEmptyLines();

            if (pos >= input.length()) break;

            char current = input[pos];

            // Preprocessor directives
            if (current == '#') {
                pos++; // Skip '#'
                skipCommentsAndEmptyLines();
                
                // Read directive name
                std::string directiveName;
                while (pos < input.length() && isAlpha(input[pos])) {
                    directiveName += input[pos++];
                }
                
                if (directiveName == "include") {
                    tokens.push_back(Token("DIRECTIVE", "#include"));
                    
                    // Skip whitespace
                    skipCommentsAndEmptyLines();
                    
                    // Process the include path
                    if (pos < input.length() && input[pos] == '<') {
                        pos++; // Skip '<'
                        std::string headerName;
                        while (pos < input.length() && input[pos] != '>') {
                            headerName += input[pos++];
                        }
                        if (pos < input.length()) pos++; // Skip '>'
                        tokens.push_back(Token("HEADER", headerName));
                    } else if (pos < input.length() && input[pos] == '"') {
                        pos++; // Skip '"'
                        std::string headerName;
                        while (pos < input.length() && input[pos] != '"') {
                            headerName += input[pos++];
                        }
                        if (pos < input.length()) pos++; // Skip '"'
                        tokens.push_back(Token("HEADER", headerName));
                    }
                }
                continue;
            }

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

            // Identifiers and keywords
            if (isAlpha(current)) {
                std::string value;
                while (pos < input.length() && isAlphaNumeric(input[pos])) {
                    value += input[pos++];
                }
                
                // Check if it's a keyword
                if (keywords.find(value) != keywords.end()) {
                    tokens.push_back(Token(keywords[value], value));
                } else {
                    tokens.push_back(Token("IDENTIFIER", value));
                }
                continue;
            }

            // Numbers
            if (isDigit(current)) {
                std::string value;
                while (pos < input.length() && isDigit(input[pos])) {
                    value += input[pos++];
                }
                tokens.push_back(Token("NUMBER", value));
                continue;
            }

            // Multi-character operators and punctuation
            if (current == '=' && pos + 1 < input.length() && input[pos + 1] == '=') {
                tokens.push_back(Token("EQUALITY", "=="));
                pos += 2;
                continue;
            }
            if (current == '!' && pos + 1 < input.length() && input[pos + 1] == '=') {
                tokens.push_back(Token("INEQUALITY", "!="));
                pos += 2;
                continue;
            }
            if (current == '<' && pos + 1 < input.length() && input[pos + 1] == '=') {
                tokens.push_back(Token("LESS_EQUAL", "<="));
                pos += 2;
                continue;
            }
            if (current == '>' && pos + 1 < input.length() && input[pos + 1] == '=') {
                tokens.push_back(Token("GREATER_EQUAL", ">="));
                pos += 2;
                continue;
            }
            if (current == '&' && pos + 1 < input.length() && input[pos + 1] == '&') {
                tokens.push_back(Token("AND", "&&"));
                pos += 2;
                continue;
            }
            if (current == '|' && pos + 1 < input.length() && input[pos + 1] == '|') {
                tokens.push_back(Token("OR", "||"));
                pos += 2;
                continue;
            }

            // Single-character operators and punctuation
            switch (current) {
                case '+': tokens.push_back(Token("PLUS", "+")); pos++; break;
                case '-': tokens.push_back(Token("MINUS", "-")); pos++; break;
                case '*': tokens.push_back(Token("MULT", "*")); pos++; break;
                case '/': tokens.push_back(Token("DIV", "/")); pos++; break;
                case '=': tokens.push_back(Token("EQUALS", "=")); pos++; break;
                case ';': tokens.push_back(Token("SEMICOLON", ";")); pos++; break;
                case ',': tokens.push_back(Token("COMMA", ",")); pos++; break;
                case '(': tokens.push_back(Token("LPAREN", "(")); pos++; break;
                case ')': tokens.push_back(Token("RPAREN", ")")); pos++; break;
                case '{': tokens.push_back(Token("LBRACE", "{")); pos++; break;
                case '}': tokens.push_back(Token("RBRACE", "}")); pos++; break;
                case '<': tokens.push_back(Token("LESS", "<")); pos++; break;
                case '>': tokens.push_back(Token("GREATER", ">")); pos++; break;
                case '!': tokens.push_back(Token("NOT", "!")); pos++; break;
                default: pos++; // Skip unrecognized characters
            }
        }

        return tokens;
    }
};

#endif