#include <iostream>
#include <fstream>
#include <sstream>
#include "lexer.h"
#include "parser.h"
#include "semantic.h"

std::string readFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filepath);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file.cpp>\n";
        return 1;
    }

    std::string filepath = argv[1];
    std::string source;

    try {
        source = readFile(filepath);
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    std::vector<Token> tokens;
    try {
        Lexer lexer(source);
        tokens = lexer.tokenize();
        std::cout << "Output from Lexical Analysis:\n";
        for (const Token& token : tokens) {
            std::cout << "  (" << token.type << ", " << token.value << ")\n";
        }
        std::cout << "\n";
    } catch (const std::exception& e) {
        std::cerr << "Lexical Analysis Error: " << e.what() << "\n";
        return 1;
    }

    ASTNode* ast = nullptr;
    try {
        Parser parser(tokens);
        ast = parser.parse();
        std::cout << "Output from Syntax Analysis (AST):\n";
        ast->print();
        std::cout << "\n";
    } catch (const std::runtime_error& e) {
        std::cerr << "Syntax Analysis Error: " << e.what() << "\n";
        delete ast;
        return 1;
    }

    std::unordered_map<std::string, std::string> symbolTable;
    try {
        semanticAnalysis(ast, symbolTable);
        std::cout << "Output from Semantic Analysis (Symbol Table):\n";
        for (const auto& [var, type] : symbolTable) {
            std::cout << "  " << var << ": " << type << "\n";
        }
    } catch (const std::runtime_error& e) {
        std::cerr << "Semantic Analysis Error: " << e.what() << "\n";
        delete ast;
        return 1;
    }

    delete ast;
    return 0;
}