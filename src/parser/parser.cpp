// src/parser/parser.cpp
#include "parser.hpp"
#include <stdexcept>

Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens) {}

std::unique_ptr<Stmt> Parser::parse() {
    std::vector<std::unique_ptr<Stmt>> statements;
    while (!isAtEnd()) {
        statements.push_back(declaration());
    }
    return std::make_unique<BlockStmt>(std::move(statements));
}

std::unique_ptr<Stmt> Parser::declaration() {
    try {
        if (match(TokenType::CLASS)) return classDeclaration();
        if (match(TokenType::DEF)) return functionDeclaration();
        if (match(TokenType::VAL) || match(TokenType::VAR)) return varDeclaration();
        return statement();
    } catch (const ParseError& error) {
        synchronize();
        return nullptr;
    }
}

std::unique_ptr<Stmt> Parser::classDeclaration() {
    Token name = consume(TokenType::IDENTIFIER, "Attendu un nom de classe.");
    std::vector<std::unique_ptr<Type>> superclasses;
    if (match(TokenType::EXTENDS)) {
        do {
            superclasses.push_back(std::make_unique<UserDefinedType>(
                consume(TokenType::IDENTIFIER, "Attendu un nom de superclasse.").lexeme));
        } while (match(TokenType::COMMA));
    }
    consume(TokenType::LBRACE, "Attendu '{' après le nom de la classe.");
    std::vector<std::unique_ptr<Stmt>> methods;
    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        methods.push_back(functionDeclaration());
    }
    consume(TokenType::RBRACE, "Attendu '}' après le corps de la classe.");
    return std::make_unique<ClassDecl>(name, std::move(superclasses), std::move(methods));
}

std::unique_ptr<Stmt> Parser::functionDeclaration() {
    Token name = consume(TokenType::IDENTIFIER, "Attendu un nom de fonction.");
    consume(TokenType::LPAREN, "Attendu '(' après le nom de la fonction.");
    std::vector<std::pair<Token, std::unique_ptr<Type>>> params;
    if (!check(TokenType::RPAREN)) {
        do {
            Token paramName = consume(TokenType::IDENTIFIER, "Attendu un nom de paramètre.");
            consume(TokenType::COLON, "Attendu ':' après le nom du paramètre.");
            params.emplace_back(paramName, type());
        } while (match(TokenType::COMMA));
    }
    consume(TokenType::RPAREN, "Attendu ')' après les paramètres.");
    std::unique_ptr<Type> returnType = nullptr;
    if (match(TokenType::COLON)) returnType = type();
    consume(TokenType::ASSIGN, "Attendu '=' après la déclaration de la fonction.");
    return std::make_unique<FunctionDecl>(name, std::move(params), std::move(returnType), blockStatement());
}

std::unique_ptr<Type> Parser::type() {
    if (match(TokenType::IDENTIFIER)) {
        Token typeName = previous();
        if (match(TokenType::LBRACKET)) {
            std::vector<std::unique_ptr<Type>> typeArgs;
            do { typeArgs.push_back(type()); } while (match(TokenType::COMMA));
            consume(TokenType::RBRACKET, "Attendu ']' après les arguments de type.");
            return std::make_unique<GenericType>(typeName.lexeme, std::move(typeArgs));
        }
        return std::make_unique<UserDefinedType>(typeName.lexeme);
    }
    if (match(TokenType::INT)) return std::make_unique<PrimitiveType>(TokenType::INT);
    if (match(TokenType::FLOAT)) return std::make_unique<PrimitiveType>(TokenType::FLOAT);
    if (match(TokenType::BOOL)) return std::make_unique<PrimitiveType>(TokenType::BOOL);
    if (match(TokenType::CHAR)) return std::make_unique<PrimitiveType>(TokenType::CHAR);
    if (match(TokenType::STRING)) return std::make_unique<PrimitiveType>(TokenType::STRING);
    if (match(TokenType::UNIT)) return std::make_unique<PrimitiveType>(TokenType::UNIT);
    throw error(peek(), "Type attendu.");
}

std::unique_ptr<Stmt> Parser::varDeclaration() {
    bool isMutable = previous().type == TokenType::VAR;
    Token name = consume(TokenType::IDENTIFIER, "Attendu un nom de variable.");
    std::unique_ptr<Type> type = nullptr;
    if (match(TokenType::COLON)) type = this->type();
    std::unique_ptr<Expr> initializer = nullptr;
    if (match(TokenType::ASSIGN)) initializer = expression();
    consume(TokenType::SEMICOLON, "Attendu ';' après la déclaration de variable.");
    return std::make_unique<VarDecl>(name, std::move(type), std::move(initializer), isMutable);
}

std::unique_ptr<Stmt> Parser::statement() {
    if (match(TokenType::IF)) return ifStatement();
    if (match(TokenType::WHILE)) return whileStatement();
    if (match(TokenType::FOR)) return forStatement();
    if (match(TokenType::RETURN)) return returnStatement();
    if (match(TokenType::LBRACE)) return blockStatement();
    return expressionStatement();
}

std::unique_ptr<Stmt> Parser::ifStatement() {
    consume(TokenType::LPAREN, "Attendu '(' après 'if'.");
    std::unique_ptr<Expr> condition = expression();
    consume(TokenType::RPAREN, "Attendu ')' après la condition.");
    std::unique_ptr<Stmt> thenBranch = statement();
    std::unique_ptr<Stmt> elseBranch = nullptr;
    if (match(TokenType::ELSE)) elseBranch = statement();
    return std::make_unique<IfStmt>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
}

std::unique_ptr<Stmt> Parser::whileStatement() {
    consume(TokenType::LPAREN, "Attendu '(' après 'while'.");
    std::unique_ptr<Expr> condition = expression();
    consume(TokenType::RPAREN, "Attendu ')' après la condition.");
    return std::make_unique<WhileStmt>(std::move(condition), statement());
}

std::unique_ptr<Stmt> Parser::forStatement() {
    consume(TokenType::LPAREN, "Attendu '(' après 'for'.");
    std::unique_ptr<Stmt> initializer;
    if (match(TokenType::SEMICOLON)) {}
    else if (match(TokenType::VAL) || match(TokenType::VAR)) initializer = varDeclaration();
    else initializer = expressionStatement();
    std::unique_ptr<Expr> condition = nullptr;
    if (!check(TokenType::SEMICOLON)) condition = expression();
    consume(TokenType::SEMICOLON, "Attendu ';' après la condition.");
    std::unique_ptr<Expr> increment = nullptr;
    if (!check(TokenType::RPAREN)) increment = expression();
    consume(TokenType::RPAREN, "Attendu ')' après la boucle for.");
    std::unique_ptr<Stmt> body = statement();
    if (increment != nullptr) {
        std::vector<std::unique_ptr<Stmt>> bodyStatements;
        bodyStatements.push_back(std::move(body));
        bodyStatements.push_back(std::make_unique<ExpressionStmt>(std::move(increment)));
        body = std::make_unique<BlockStmt>(std::move(bodyStatements));
    }
    if (condition == nullptr) condition = std::make_unique<LiteralExpr>(true);
    if (initializer != nullptr) {
        std::vector<std::unique_ptr<Stmt>> statements;
        statements.push_back(std::move(initializer));
        statements.push_back(std::make_unique<WhileStmt>(std::move(condition), std::move(body)));
        return std::make_unique<BlockStmt>(std::move(statements));
    }
    return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

std::unique_ptr<Stmt> Parser::blockStatement() {
    std::vector<std::unique_ptr<Stmt>> statements;
    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        statements.push_back(declaration());
    }
    consume(TokenType::RBRACE, "Attendu '}' après le bloc.");
    return std::make_unique<BlockStmt>(std::move(statements));
}

std::unique_ptr<Stmt> Parser::expressionStatement() {
    std::unique_ptr<Expr> expr = expression();
    consume(TokenType::SEMICOLON, "Attendu ';' après l'expression.");
    return std::make_unique<ExpressionStmt>(std::move(expr));
}

std::unique_ptr<Stmt> Parser::returnStatement() {
    Token keyword = previous();
    std::unique_ptr<Expr> value = nullptr;
    if (!check(TokenType::SEMICOLON)) value = expression();
    consume(TokenType::SEMICOLON, "Attendu ';' après 'return'.");
    return std::make_unique<ReturnStmt>(keyword, std::move(value));
}

std::unique_ptr<Expr> Parser::expression() { return assignment(); }
std::unique_ptr<Expr> Parser::assignment() {
    std::unique_ptr<Expr> expr = orExpr();
    if (match(TokenType::ASSIGN)) {
        Token equals = previous();
        std::unique_ptr<Expr> value = assignment();
        if (auto var = dynamic_cast<VariableExpr*>(expr.get())) {
            return std::make_unique<SetExpr>(std::make_unique<ThisExpr>(equals), var->name, std::move(value));
        }
        throw error(equals, "Cible d'affectation invalide.");
    }
    return expr;
}

std::unique_ptr<Expr> Parser::orExpr() {
    std::unique_ptr<Expr> expr = andExpr();
    while (match(TokenType::OR)) {
        Token op = previous();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, andExpr());
    }
    return expr;
}

std::unique_ptr<Expr> Parser::andExpr() {
    std::unique_ptr<Expr> expr = equality();
    while (match(TokenType::AND)) {
        Token op = previous();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, equality());
    }
    return expr;
}

std::unique_ptr<Expr> Parser::equality() {
    std::unique_ptr<Expr> expr = comparison();
    while (match(TokenType::EQ) || match(TokenType::NEQ)) {
        Token op = previous();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, comparison());
    }
    return expr;
}

std::unique_ptr<Expr> Parser::comparison() {
    std::unique_ptr<Expr> expr = term();
    while (match(TokenType::LT) || match(TokenType::GT) || match(TokenType::LE) || match(TokenType::GE)) {
        Token op = previous();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, term());
    }
    return expr;
}

std::unique_ptr<Expr> Parser::term() {
    std::unique_ptr<Expr> expr = factor();
    while (match(TokenType::PLUS) || match(TokenType::MINUS)) {
        Token op = previous();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, factor());
    }
    return expr;
}

std::unique_ptr<Expr> Parser::factor() {
    std::unique_ptr<Expr> expr = unary();
    while (match(TokenType::MULT) || match(TokenType::DIV) || match(TokenType::MOD)) {
        Token op = previous();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, unary());
    }
    return expr;
}

std::unique_ptr<Expr> Parser::unary() {
    if (match(TokenType::NOT) || match(TokenType::MINUS)) {
        Token op = previous();
        return std::make_unique<UnaryExpr>(op, unary());
    }
    return call();
}

std::unique_ptr<Expr> Parser::call() {
    std::unique_ptr<Expr> expr = primary();
    while (true) {
        if (match(TokenType::LPAREN)) {
            expr = finishCall(std::move(expr));
        } else if (match(TokenType::DOT)) {
            Token name = consume(TokenType::IDENTIFIER, "Attendu un nom de propriété après '.'.");
            expr = std::make_unique<GetExpr>(std::move(expr), name);
        } else break;
    }
    return expr;
}

std::unique_ptr<Expr> Parser::finishCall(std::unique_ptr<Expr> callee) {
    std::vector<std::unique_ptr<Expr>> arguments;
    if (!check(TokenType::RPAREN)) {
        do {
            if (arguments.size() >= 255) throw error(peek(), "Trop d'arguments");
            arguments.push_back(expression());
        } while (match(TokenType::COMMA));
    }
    consume(TokenType::RPAREN, "Attendu ')' après les arguments.");
    return std::make_unique<CallExpr>(std::move(callee), std::move(arguments));
}

std::unique_ptr<Expr> Parser::primary() {
    if (match(TokenType::TRUE)) return std::make_unique<LiteralExpr>(true);
    if (match(TokenType::FALSE)) return std::make_unique<LiteralExpr>(false);
    if (match(TokenType::NULL_LITERAL)) return std::make_unique<LiteralExpr>(nullptr);
    if (match(TokenType::INT_LITERAL) || match(TokenType::FLOAT_LITERAL)) {
        return std::make_unique<LiteralExpr>(previous().literal);
    }
    if (match(TokenType::STRING_LITERAL) || match(TokenType::CHAR_LITERAL)) {
        return std::make_unique<LiteralExpr>(previous().literal);
    }
    if (match(TokenType::IDENTIFIER)) return std::make_unique<VariableExpr>(previous());
    if (match(TokenType::THIS)) return std::make_unique<ThisExpr>(previous());
    if (match(TokenType::LPAREN)) {
        std::unique_ptr<Expr> expr = expression();
        consume(TokenType::RPAREN, "Attendu ')' après l'expression.");
        return std::make_unique<GroupingExpr>(std::move(expr));
    }
    throw error(peek(), "Expression attendue.");
}

Token Parser::peek() const { return tokens[current]; }
Token Parser::previous() const { return tokens[current - 1]; }
bool Parser::isAtEnd() const { return peek().type == TokenType::EOF_TOKEN; }
bool Parser::check(TokenType type) const { return !isAtEnd() && peek().type == type; }
bool Parser::match(TokenType type) { return check(type) && (++current, true); }
Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) return advance();
    throw error(peek(), message);
}
Token Parser::advance() { if (!isAtEnd()) current++; return previous(); }
Parser::ParseError Parser::error(const Token& token, const std::string& message) {
    return ParseError("[Ligne " + std::to_string(token.line) + "] Erreur: " + message);
}
void Parser::synchronize() {
    advance();
    while (!isAtEnd()) {
        if (previous().type == TokenType::SEMICOLON) return;
        switch (peek().type) {
            case TokenType::CLASS: case TokenType::DEF: case TokenType::VAL:
            case TokenType::VAR: case TokenType::IF: case TokenType::WHILE:
            case TokenType::FOR: case TokenType::RETURN: return;
            default: advance();
        }
    }
}