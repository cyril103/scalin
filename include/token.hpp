// include/token.hpp
#pragma once
#include <string>
#include <variant>

enum class TokenType {
    OBJECT, DEF, VAL, VAR, IF, ELSE, MATCH, CASE, CLASS, TRAIT,
    EXTENDS, WITH, RETURN, WHILE, FOR, TRUE, FALSE, NULL_LITERAL,
    INT, FLOAT, BOOL, CHAR, STRING, UNIT, OPTION, LIST, ARRAY,
    INT_LITERAL, FLOAT_LITERAL, BOOL_LITERAL, CHAR_LITERAL, STRING_LITERAL,
    IDENTIFIER,
    PLUS, MINUS, MULT, DIV, MOD,
    EQ, NEQ, LT, GT, LE, GE,
    AND, OR, NOT,
    ASSIGN, PLUS_ASSIGN, MINUS_ASSIGN, MULT_ASSIGN, DIV_ASSIGN,
    LPAREN, RPAREN, LBRACE, RBRACE, LBRACKET, RBRACKET,
    COLON, SEMICOLON, COMMA, DOT, ARROW, DOUBLE_ARROW,
    EOF_TOKEN
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int column;
    std::variant<int64_t, double, bool, char, std::string> literal;

    Token(TokenType type, std::string lexeme, int line, int column)
        : type(type), lexeme(std::move(lexeme)), line(line), column(column) {} 
    
    Token(TokenType type, std::string lexeme, int line, int column, 
          std::variant<int64_t, double, bool, char, std::string> literal)
        : type(type), lexeme(std::move(lexeme)), line(line), column(column), literal(literal) {}
};