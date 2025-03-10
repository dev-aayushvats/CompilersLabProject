#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>
#include "lexer.h" // Assumes lexer function is defined here

// Function to compile code to JS (currently just returns tokens)
std::vector<Token> compileToJS(const std::string &code)
{
    std::vector<Token> tokens;

    try
    {
        tokens = lexer(code); // Call the lexer function
    }
    catch (const std::exception &error)
    {
        throw std::runtime_error("Lexer stage error: " + std::string(error.what()));
    }

    return tokens;
}

// Main function
void main_function()
{ // Renamed to avoid conflict with standard `main`
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

        // Compile to JS (currently just tokenizes)
        std::vector<Token> jsCode = compileToJS(cppCode);

        // Optional: Print tokens for verification (since the JS code doesn't use jsCode yet)
        for (const Token &t : jsCode)
        {
            std::cout << "(" << t.type << ", " << t.value << ") ";
        }
        std::cout << std::endl;
    }
    catch (const std::exception &error)
    {
        std::cerr << "Error: " << error.what() << std::endl;
    }
}

int main()
{
    main_function();
    return 0;
}