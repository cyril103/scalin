// src/main.cpp
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "codegen/codegen.hpp"
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

void writeFile(const std::string& path, const std::string& content) {
    std::ofstream file(path);
    if (!file) throw std::runtime_error("Impossible d'écrire dans le fichier: " + path);
    file << content;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input.sc> <output.asm>\n";
        return 1;
    }

    try {
        std::string inputPath = argv[1];
        std::string outputPath = argv[2];

        std::cout << "Compilation de: " << inputPath << "\n";

        // Étape 1: Lecture du fichier source
        std::cout << "[1/4] Lecture du fichier...\n";
        std::string source = readFile(inputPath);

        // Étape 2: Tokenisation
        std::cout << "[2/4] Tokenisation...\n";
        Lexer lexer(source);
        std::vector<Token> tokens = lexer.tokenize();
        std::cout << "   Tokens générés: " << tokens.size() << "\n";

        // Étape 3: Parsing
        std::cout << "[3/4] Parsing...\n";
        Parser parser(tokens);
        std::unique_ptr<Stmt> ast = parser.parse();
        std::cout << "   AST généré\n";

        // Étape 4: Génération de code assembleur
        std::cout << "[4/4] Génération de code assembleur...\n";
        CodeGenerator codegen;
        std::string asmCode = codegen.generate(ast);

        // Écrire le code assembleur dans le fichier de sortie
        writeFile(outputPath, asmCode);
        std::cout << "   Code assembleur généré: " << outputPath << "\n";

        std::cout << "Compilation terminée avec succès!\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Erreur: " << e.what() << "\n";
        return 1;
    }
}