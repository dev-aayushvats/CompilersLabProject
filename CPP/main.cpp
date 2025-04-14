#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
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

// Helper function to count specific node types in the AST
int countNodeTypes(ASTNode* node, const std::string& type) {
    if (!node) return 0;
    
    int count = (node->type == type) ? 1 : 0;
    
    for (ASTNode* child : node->children) {
        count += countNodeTypes(child, type);
    }
    
    return count;
}

// Helper to print summary of recognized constructs
void printSummary(ASTNode* ast) {
    std::cout << "\nCompilation Summary:\n";
    std::cout << "-------------------\n";
    std::cout << "Includes: " << countNodeTypes(ast, "INCLUDE") << "\n";
    std::cout << "Functions: " << countNodeTypes(ast, "FUNCTION") << "\n";
    std::cout << "Variable Declarations: " 
              << countNodeTypes(ast, "DECLARATION_INT") + countNodeTypes(ast, "DECLARATION_CHAR_TYPE") << "\n";
    std::cout << "Assignments: " << countNodeTypes(ast, "ASSIGNMENT") << "\n";
    std::cout << "If Statements: " << countNodeTypes(ast, "IF") << "\n";
    std::cout << "While Loops: " << countNodeTypes(ast, "WHILE") << "\n";
    std::cout << "For Loops: " << countNodeTypes(ast, "FOR") << "\n";
    std::cout << "Return Statements: " << countNodeTypes(ast, "RETURN") << "\n";
    std::cout << "-------------------\n";
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
        std::cout << "Successfully read file: " << filepath << "\n\n";
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    std::vector<Token> tokens;
    try {
        Lexer lexer(source);
        tokens = lexer.tokenize();
        std::cout << "Lexical Analysis Results:\n";
        std::cout << "========================\n";
        
        // Group tokens by type for easier reading
        std::unordered_map<std::string, int> tokenCounts;
        for (const Token& token : tokens) {
            tokenCounts[token.type]++;
        }
        
        // Print token counts by type
        std::cout << "Token Type Counts:\n";
        for (const auto& [type, count] : tokenCounts) {
            std::cout << "  " << std::setw(15) << std::left << type << ": " << count << "\n";
        }
        
        // Print first 20 tokens as sample
        std::cout << "\nSample Tokens (first 20):\n";
        for (size_t i = 0; i < std::min(tokens.size(), size_t(20)); ++i) {
            std::cout << "  (" << tokens[i].type << ", \"" << tokens[i].value << "\")\n";
        }
        
        if (tokens.size() > 20) {
            std::cout << "  ... and " << (tokens.size() - 20) << " more tokens\n";
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
        std::cout << "Syntax Analysis Results:\n";
        std::cout << "======================\n";
        ast->print();
        
        // Print summary of constructs found
        printSummary(ast);
        
        std::cout << "\n";
    } catch (const std::runtime_error& e) {
        std::cerr << "Syntax Analysis Error: " << e.what() << "\n";
        delete ast;
        return 1;
    }

    std::unordered_map<std::string, std::string> symbolTable;
    try {
        semanticAnalysis(ast, symbolTable);
        std::cout << "Semantic Analysis Results:\n";
        std::cout << "========================\n";
        
        if (symbolTable.empty()) {
            std::cout << "No symbols defined in the program.\n";
        } else {
            std::cout << "Symbol Table:\n";
            for (const auto& [var, type] : symbolTable) {
                std::cout << "  " << std::setw(15) << std::left << var << ": " << type << "\n";
            }
        }
    } catch (const std::runtime_error& e) {
        std::cerr << "Semantic Analysis Error: " << e.what() << "\n";
        delete ast;
        return 1;
    }

    std::cout << "\nCompilation completed successfully.\n";
    delete ast;
    return 0;
}