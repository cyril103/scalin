// src/main.cpp
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) throw std::runtime_error("Impossible d'ouvrir le fichier: " + path);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input.sc>\n";
        return 1;
    }

    try {
        std::string source = readFile(argv[1]);
        std::cout << "Compilation de: " << argv[1] << "\n";
        std::cout << "[1/4] Tokenisation...\n";
        Lexer lexer(source);
        std::vector<Token> tokens = lexer.tokenize();
        std::cout << "   Tokens générés: " << tokens.size() << "\n";
        std::cout << "[2/4] Parsing...\n";
        Parser parser(tokens);
        std::unique_ptr<Stmt> ast = parser.parse();
        std::cout << "   AST généré\n";
        std::cout << "Compilation terminée avec succès!\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Erreur: " << e.what() << "\n";
        return 1;
    }
}