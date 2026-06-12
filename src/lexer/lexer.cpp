// src/lexer/lexer.cpp
#include "lexer.hpp"
#include <cctype>
#include <stdexcept>
#include <unordered_map>

Lexer::Lexer(const std::string& source) : source(source) {}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (!isAtEnd()) {
        char c = advance();
        switch (c) {
            case ' ': case '\t': case '\r': break;
            case '\n': line++; column = 1; break;
            case '+':
                tokens.push_back(makeToken(match('=') ? TokenType::PLUS_ASSIGN : TokenType::PLUS));
                break;
            case '-':
                tokens.push_back(makeToken(match('=') ? TokenType::MINUS_ASSIGN : TokenType::MINUS));
                break;
            case '*':
                tokens.push_back(makeToken(match('=') ? TokenType::MULT_ASSIGN : TokenType::MULT));
                break;
            case '/':
                tokens.push_back(makeToken(match('=') ? TokenType::DIV_ASSIGN : TokenType::DIV));
                break;
            case '%': tokens.push_back(makeToken(TokenType::MOD)); break;
            case '(': tokens.push_back(makeToken(TokenType::LPAREN)); break;
            case ')': tokens.push_back(makeToken(TokenType::RPAREN)); break;
            case '{': tokens.push_back(makeToken(TokenType::LBRACE)); break;
            case '}': tokens.push_back(makeToken(TokenType::RBRACE)); break;
            case '[': tokens.push_back(makeToken(TokenType::LBRACKET)); break;
            case ']': tokens.push_back(makeToken(TokenType::RBRACKET)); break;
            case ':': tokens.push_back(makeToken(TokenType::COLON)); break;
            case ';': tokens.push_back(makeToken(TokenType::SEMICOLON)); break;
            case ',': tokens.push_back(makeToken(TokenType::COMMA)); break;
            case '.': tokens.push_back(makeToken(TokenType::DOT)); break;
            case '=':
                tokens.push_back(makeToken(match('>') ? TokenType::DOUBLE_ARROW : TokenType::ASSIGN));
                break;
            case '!':
                tokens.push_back(makeToken(match('=') ? TokenType::NEQ : TokenType::NOT));
                break;
            case '<':
                tokens.push_back(makeToken(match('=') ? TokenType::LE : TokenType::LT));
                break;
            case '>':
                tokens.push_back(makeToken(match('=') ? TokenType::GE : TokenType::GT));
                break;
            case '&':
                if (match('&')) tokens.push_back(makeToken(TokenType::AND));
                else throw std::runtime_error("Opérateur '&' non supporté seul");
                break;
            case '|':
                if (match('|')) tokens.push_back(makeToken(TokenType::OR));
                else throw std::runtime_error("Opérateur '|' non supporté seul");
                break;
            case '"': tokens.push_back(string()); break;
            case '\'': tokens.push_back(character()); break;
            default:
                if (isdigit(c)) tokens.push_back(number());
                else if (isalpha(c) || c == '_') tokens.push_back(identifier());
                else throw std::runtime_error(std::string("Caractère inattendu: ") + c);
        }
    }
    tokens.push_back(Token(TokenType::EOF_TOKEN, "", line, column));
    return tokens;
}

char Lexer::advance() { column++; return source[current++]; }
bool Lexer::match(char expected) {
    if (isAtEnd() || source[current] != expected) return false;
    current++; column++; return true;
}
char Lexer::peek() const { return isAtEnd() ? '\0' : source[current]; }
char Lexer::peekNext() const { return current + 1 >= source.size() ? '\0' : source[current + 1]; }
bool Lexer::isAtEnd() const { return current >= source.size(); }

Token Lexer::makeToken(TokenType type) const {
    return Token(type, std::string(1, source[current - 1]), line, column - 1);
}
Token Lexer::makeToken(TokenType type, std::string lexeme) const {
    return Token(type, lexeme, line, column - lexeme.size());
}

Token Lexer::number() {
    size_t start = current - 1;
    while (isdigit(peek())) advance();
    if (peek() == '.' && isdigit(peekNext())) {
        advance();
        while (isdigit(peek())) advance();
        return Token(TokenType::FLOAT_LITERAL, source.substr(start, current - start), line, start + 1,
                     std::stod(source.substr(start, current - start)));
    }
    return Token(TokenType::INT_LITERAL, source.substr(start, current - start), line, start + 1,
                 std::stoll(source.substr(start, current - start)));
}

Token Lexer::identifier() {
    size_t start = current - 1;
    while (isalnum(peek()) || peek() == '_') advance();
    std::string text = source.substr(start, current - start);
    static const std::unordered_map<std::string, TokenType> keywords = {
        {"object", TokenType::OBJECT}, {"def", TokenType::DEF}, {"val", TokenType::VAL},
        {"var", TokenType::VAR}, {"if", TokenType::IF}, {"else", TokenType::ELSE},
        {"match", TokenType::MATCH}, {"case", TokenType::CASE}, {"class", TokenType::CLASS},
        {"trait", TokenType::TRAIT}, {"extends", TokenType::EXTENDS}, {"with", TokenType::WITH},
        {"return", TokenType::RETURN}, {"while", TokenType::WHILE}, {"for", TokenType::FOR},
        {"true", TokenType::TRUE}, {"false", TokenType::FALSE}, {"null", TokenType::NULL_LITERAL},
        {"Int", TokenType::INT}, {"Float", TokenType::FLOAT}, {"Bool", TokenType::BOOL},
        {"Char", TokenType::CHAR}, {"String", TokenType::STRING}, {"Unit", TokenType::UNIT},
        {"Option", TokenType::OPTION}, {"List", TokenType::LIST}, {"Array", TokenType::ARRAY}
    };
    if (keywords.find(text) != keywords.end())
        return Token(keywords.at(text), text, line, start + 1);
    return Token(TokenType::IDENTIFIER, text, line, start + 1);
}

Token Lexer::string() {
    size_t start = current;
    while (peek() != '"' && !isAtEnd()) advance();
    if (isAtEnd()) throw std::runtime_error("String non terminé");
    advance();
    std::string value = source.substr(start, current - start - 1);
    return Token(TokenType::STRING_LITERAL, value, line, start + 1, value);
}

Token Lexer::character() {
    size_t start = current;
    char value;
    if (peek() == '\\') {
        advance();
        switch (peek()) {
            case 'n': value = '\n'; break; case 't': value = '\t'; break;
            case 'r': value = '\r'; break; case '0': value = '\0'; break;
            case '\'': value = '\''; break; case '"': value = '"'; break;
            case '\\': value = '\\'; break;
            default: throw std::runtime_error("Échappement invalide");
        }
        advance();
    } else {
        value = peek(); advance();
    }
    if (source[current - 1] != '\'') throw std::runtime_error("Caractère non terminé");
    return Token(TokenType::CHAR_LITERAL, std::string(1, value), line, start + 1, value);
}