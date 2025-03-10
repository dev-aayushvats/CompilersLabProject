#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>
#include "lexer.h"    // For lexer function
#include "parser.h"   // For Parser class
#include "semantic.h" // For semanticAnalysis function

// Function to run lexer, parser, and semantic analysis
std::vector<Token> runLexerParserSemantic(const std::string &code)
{
    std::vector<Token> tokens;
    ASTNode *ast = nullptr;
    std::map<std::string, std::string> symbolTable;

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
        ast = parser.parse();
    }
    catch (const std::exception &error)
    {
        throw std::runtime_error("Parser stage error: " + std::string(error.what()));
    }

    // Semantic analysis stage
    try
    {
        symbolTable = semanticAnalysis(ast);
        // Print symbol table for verification
        std::cout << "Symbol Table after Semantic Analysis:\n";
        for (const auto &pair : symbolTable)
        {
            std::cout << pair.first << ": " << pair.second << "\n";
        }
    }
    catch (const std::exception &error)
    {
        throw std::runtime_error("Semantic Analysis stage error: " + std::string(error.what()));
    }

    delete ast;    // Clean up AST memory
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

        // Run lexer, parser, and semantic analysis
        std::vector<Token> tokens = runLexerParserSemantic(cppCode);

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