// src/main.cpp
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "codegen/codegen.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdlib> // Pour system()
#include <cstring> // Pour strerror
#include <string>

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

// Exécute une commande shell et vérifie son succès
void runCommand(const std::string& cmd) {
    int result = system(cmd.c_str());
    if (result != 0) {
        throw std::runtime_error("Commande échouée: " + cmd + " (nasm/ld non installé ?)");
    }
}

// Vérifie si une commande existe
bool commandExists(const std::string& cmd) {
    return system(("command -v " + cmd + " >/dev/null 2>&1").c_str()) == 0;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input.sc>\n";
        return 1;
    }

    try {
        std::string inputPath = argv[1];
        std::string asmPath = inputPath + ".asm";
        std::string objPath = inputPath + ".o";
        std::string outputPath = inputPath.substr(0, inputPath.find_last_of('.'));

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

        // Écrire le code assembleur dans le fichier temporaire
        writeFile(asmPath, asmCode);
        std::cout << "   Code assembleur généré: " << asmPath << "\n";

        // Vérifier si NASM et LD sont disponibles
        bool hasNasm = commandExists("nasm");
        bool hasLd = commandExists("ld");

        if (hasNasm && hasLd) {
            // Étape 5: Assemblage avec NASM
            std::cout << "[5/4] Assemblage avec NASM...\n";
            runCommand("nasm -f elf64 " + asmPath + " -o " + objPath);

            // Étape 6: Linking avec LD
            std::cout << "[6/4] Linking avec LD...\n";
            runCommand("ld -o " + outputPath + " " + objPath);

            // Nettoyer les fichiers temporaires
            std::remove(asmPath.c_str());
            std::remove(objPath.c_str());

            std::cout << "Compilation terminée avec succès! Exécutable: " << outputPath << "\n";
        } else {
            std::cout << "NASM ou LD non trouvé. Fichier assembleur généré: " << asmPath << "\n";
            std::cout << "Pour générer l'exécutable, installez NASM et LD et relancez la commande.\n";
        }
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Erreur: " << e.what() << "\n";
        return 1;
    }
}