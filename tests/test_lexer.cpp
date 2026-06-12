#include "../src/lexer/lexer.hpp"
#include <cassert>
#include <iostream>
#include <any>

int main() {
    std::cout << "Testing lexer...\n";
    
    // Test integers
    {
        std::string source = "42";
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        assert(tokens.size() >= 1);
        assert(tokens[0].type == TokenType::INT_LITERAL);
        std::cout << "  Integers: OK\n";
    }
    
    // Test floats
    {
        std::string source = "3.14";
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        assert(tokens.size() >= 1);
        assert(tokens[0].type == TokenType::FLOAT_LITERAL);
        std::cout << "  Floats: OK\n";
    }
    
    // Test strings
    {
        std::string source = "\"hello\"";
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        assert(tokens.size() >= 1);
        assert(tokens[0].type == TokenType::STRING_LITERAL);
        std::cout << "  Strings: OK\n";
    }
    
    // Test keywords
    {
        std::string source = "object def val if else";
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        assert(tokens[0].type == TokenType::OBJECT);
        assert(tokens[1].type == TokenType::DEF);
        assert(tokens[2].type == TokenType::VAL);
        std::cout << "  Keywords: OK\n";
    }
    
    // Test operators
    {
        std::string source = "+ - * / % == != < >";
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        assert(tokens[0].type == TokenType::PLUS);
        assert(tokens[1].type == TokenType::MINUS);
        assert(tokens[2].type == TokenType::MULT);
        std::cout << "  Operators: OK\n";
    }
    
    std::cout << "All lexer tests passed!\n";
    return 0;
}
