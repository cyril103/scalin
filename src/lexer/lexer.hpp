// src/lexer/lexer.hpp
#pragma once
#include "token.hpp"
#include <string>
#include <vector>

class Lexer {
public:
    explicit Lexer(const std::string& source);
    std::vector<Token> tokenize();

private:
    const std::string source;
    size_t current = 0;
    size_t line = 1;
    size_t column = 1;

    char advance();
    char peek() const;
    char peekNext() const;
    bool isAtEnd() const;
    bool match(char expected);
    Token makeToken(TokenType type) const;
    Token makeToken(TokenType type, std::string lexeme) const;
    Token identifier();
    Token number();
    Token string();
    Token character();
};