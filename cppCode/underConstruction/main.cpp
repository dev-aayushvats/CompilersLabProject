#include <iostream>
#include <fstream>
#include <stdexcept>
#include "lexer.h"
#include "parser.h"
#include "semantic.h"
#include "codegen.h"

std::string compileToJS(const std::string &code)
{
    std::vector<Token> tokens;
    ASTNode *ast = nullptr;
    std::map<std::string, std::string> symbolTable;
    std::string jsCode;

    try
    {
        tokens = lexer(code);
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error("Lexer stage error: " + std::string(e.what()));
    }

    try
    {
        Parser parser(tokens);
        ast = parser.parse();
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error("Parser stage error: " + std::string(e.what()));
    }

    try
    {
        symbolTable = semanticAnalysis(ast);
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error("Semantic Analysis stage error: " + std::string(e.what()));
    }

    try
    {
        CodeGenerator codegen;
        jsCode = codegen.generate(ast);
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error("Code Generation stage error: " + std::string(e.what()));
    }

    delete ast; // Clean up AST
    return jsCode;
}

int main()
{
    try
    {
        std::ifstream file("input.cpp");
        if (!file.is_open())
            throw std::runtime_error("Failed to open input.cpp");
        std::string cppCode((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();

        std::string jsCode = compileToJS(cppCode);
        std::cout << "\nGenerated JavaScript code:\n"
                  << jsCode << std::endl;

        // Note: C++ cannot directly execute JavaScript like `eval` in JS.
        // You could integrate a JS engine (e.g., V8) to run it, but that's beyond this scope.
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}