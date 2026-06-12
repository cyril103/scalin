#include "lexer/lexer.hpp"
#include <cassert>
#include <iostream>

int main() {
    std::cout << "Testing lexer...
";
    
    // Test integers
    {
        std::string source = "42 123 -456";
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        assert(tokens.size() == 4);
        assert(tokens[0].type == TokenType::INT_LITERAL);
        assert(std::any_cast<int64_t>(tokens[0].literal) == 42);
        std::cout << "  Integers: OK
";
    }
    
    // Test floats
    {
        std::string source = "3.14 0.5";
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        assert(tokens.size() == 3);
        assert(tokens[0].type == TokenType::FLOAT_LITERAL);
        std::cout << "  Floats: OK
";
    }
    
    // Test strings
    {
        std::string source = ""hello" "world"";
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        assert(tokens.size() == 3);
        assert(tokens[0].type == TokenType::STRING_LITERAL);
        std::cout << "  Strings: OK
";
    }
    
    // Test keywords
    {
        std::string source = "object def val if else";
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        assert(tokens[0].type == TokenType::OBJECT);
        assert(tokens[1].type == TokenType::DEF);
        assert(tokens[2].type == TokenType::VAL);
        std::cout << "  Keywords: OK
";
    }
    
    // Test operators
    {
        std::string source = "+ - * / % == != < >";
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        assert(tokens[0].type == TokenType::PLUS);
        assert(tokens[1].type == TokenType::MINUS);
        assert(tokens[2].type == TokenType::MULT);
        std::cout << "  Operators: OK
";
    }
    
    std::cout << "All lexer tests passed!
";
    return 0;
}