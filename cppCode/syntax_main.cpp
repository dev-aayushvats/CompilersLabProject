#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>
#include "lexer.h"  // For lexer function
#include "parser.h" // For Parser class

// Function to run lexer and parser
std::vector<Token> runLexerAndParser(const std::string &code)
{
    std::vector<Token> tokens;

    // Lexer stage
    try
    {
        tokens = lexer(code);
    }
    catch (const std::exception &error)
    {
        throw std::runtime_error("Lexer stage error: " + std::string(error.what()));
    }

    // Parser stage
    try
    {
        Parser parser(tokens);
        ASTNode *ast = parser.parse();
        delete ast; // Clean up AST memory (no further use here)
    }
    catch (const std::exception &error)
    {
        throw std::runtime_error("Parser stage error: " + std::string(error.what()));
    }

    return tokens; // Return tokens for optional verification
}

int main()
{
    try
    {
        // Read the input.cpp file
        std::ifstream file("input.cpp");
        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open input.cpp");
        }

        // Read file contents into a string
        std::string cppCode((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();

        // Run lexer and parser
        std::vector<Token> tokens = runLexerAndParser(cppCode);

        // Optional: Print tokens for verification
        std::cout << "Tokens after processing:\n";
        for (const Token &t : tokens)
        {
            std::cout << "(" << t.type << ", " << t.value << ") ";
        }
        std::cout << std::endl;
    }
    catch (const std::exception &error)
    {
        std::cerr << "Error: " << error.what() << std::endl;
    }

    return 0;
}