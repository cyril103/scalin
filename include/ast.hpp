// include/ast.hpp
#pragma once
#include "token.hpp"
#include <memory>
#include <vector>
#include <string>
#include <any>

class ASTVisitor;

class Expr {
public:
    virtual ~Expr() = default;
    virtual std::any accept(ASTVisitor& visitor) = 0;
};

class Stmt {
public:
    virtual ~Stmt() = default;
    virtual std::any accept(ASTVisitor& visitor) = 0;
};

class Type {
public:
    virtual ~Type() = default;
    virtual std::string toString() const = 0;
    virtual std::unique_ptr<Type> clone() const = 0;
};

class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;
    virtual std::any visitLiteralExpr(Expr& expr) = 0;
    virtual std::any visitBinaryExpr(Expr& expr) = 0;
    virtual std::any visitUnaryExpr(Expr& expr) = 0;
    virtual std::any visitVariableExpr(Expr& expr) = 0;
    virtual std::any visitGroupingExpr(Expr& expr) = 0;
    virtual std::any visitCallExpr(Expr& expr) = 0;
    virtual std::any visitGetExpr(Expr& expr) = 0;
    virtual std::any visitSetExpr(Expr& expr) = 0;
    virtual std::any visitThisExpr(Expr& expr) = 0;
    virtual std::any visitSuperExpr(Expr& expr) = 0;
    virtual std::any visitVarDecl(Stmt& stmt) = 0;
    virtual std::any visitFunctionDecl(Stmt& stmt) = 0;
    virtual std::any visitClassDecl(Stmt& stmt) = 0;
    virtual std::any visitExpressionStmt(Stmt& stmt) = 0;
    virtual std::any visitIfStmt(Stmt& stmt) = 0;
    virtual std::any visitBlockStmt(Stmt& stmt) = 0;
    virtual std::any visitWhileStmt(Stmt& stmt) = 0;
    virtual std::any visitReturnStmt(Stmt& stmt) = 0;
};

class PrimitiveType : public Type {
public:
    TokenType kind;
    explicit PrimitiveType(TokenType kind) : kind(kind) {} 
    std::string toString() const override {
        switch (kind) {
            case TokenType::INT: return "Int";
            case TokenType::FLOAT: return "Float";
            case TokenType::BOOL: return "Bool";
            case TokenType::CHAR: return "Char";
            case TokenType::STRING: return "String";
            case TokenType::UNIT: return "Unit";
            default: return "Unknown";
        }
    }
    std::unique_ptr<Type> clone() const override {
        return std::make_unique<PrimitiveType>(kind);
    }
};

class UserDefinedType : public Type {
public:
    std::string name;
    explicit UserDefinedType(std::string name) : name(std::move(name)) {} 
    std::string toString() const override { return name; }
    std::unique_ptr<Type> clone() const override {
        return std::make_unique<UserDefinedType>(name);
    }
};

class GenericType : public Type {
public:
    std::string baseName;
    std::vector<std::unique_ptr<Type>> typeArgs;
    GenericType(std::string baseName, std::vector<std::unique_ptr<Type>> typeArgs)
        : baseName(std::move(baseName)), typeArgs(std::move(typeArgs)) {} 
    std::string toString() const override {
        std::string result = baseName + "[";
        for (size_t i = 0; i < typeArgs.size(); ++i) {
            if (i > 0) result += ", ";
            result += typeArgs[i]->toString();
        }
        result += "]";
        return result;
    }
    std::unique_ptr<Type> clone() const override {
        std::vector<std::unique_ptr<Type>> clonedArgs;
        for (const auto& arg : typeArgs) {
            clonedArgs.push_back(arg->clone());
        }
        return std::make_unique<GenericType>(baseName, std::move(clonedArgs));
    }
};

class LiteralExpr : public Expr {
public:
    std::any value;
    explicit LiteralExpr(std::any value) : value(std::move(value)) {} 
    std::any accept(ASTVisitor& visitor) override { return visitor.visitLiteralExpr(*this); }
};

class BinaryExpr : public Expr {
public:
    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;
    BinaryExpr(std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right)
        : left(std::move(left)), op(op), right(std::move(right)) {} 
    std::any accept(ASTVisitor& visitor) override { return visitor.visitBinaryExpr(*this); }
};

class UnaryExpr : public Expr {
public:
    Token op;
    std::unique_ptr<Expr> right;
    UnaryExpr(Token op, std::unique_ptr<Expr> right)
        : op(op), right(std::move(right)) {} 
    std::any accept(ASTVisitor& visitor) override { return visitor.visitUnaryExpr(*this); }
};

class VariableExpr : public Expr {
public:
    Token name;
    explicit VariableExpr(Token name) : name(name) {} 
    std::any accept(ASTVisitor& visitor) override { return visitor.visitVariableExpr(*this); }
};

class GroupingExpr : public Expr {
public:
    std::unique_ptr<Expr> expression;
    explicit GroupingExpr(std::unique_ptr<Expr> expression)
        : expression(std::move(expression)) {} 
    std::any accept(ASTVisitor& visitor) override { return visitor.visitGroupingExpr(*this); }
};

class CallExpr : public Expr {
public:
    std::unique_ptr<Expr> callee;
    std::vector<std::unique_ptr<Expr>> arguments;
    CallExpr(std::unique_ptr<Expr> callee, std::vector<std::unique_ptr<Expr>> arguments)
        : callee(std::move(callee)), arguments(std::move(arguments)) {} 
    std::any accept(ASTVisitor& visitor) override { return visitor.visitCallExpr(*this); }
};

class GetExpr : public Expr {
public:
    std::unique_ptr<Expr> object;
    Token name;
    GetExpr(std::unique_ptr<Expr> object, Token name)
        : object(std::move(object)), name(name) {} 
    std::any accept(ASTVisitor& visitor) override { return visitor.visitGetExpr(*this); }
};

class SetExpr : public Expr {
public:
    std::unique_ptr<Expr> object;
    Token name;
    std::unique_ptr<Expr> value;
    SetExpr(std::unique_ptr<Expr> object, Token name, std::unique_ptr<Expr> value)
        : object(std::move(object)), name(name), value(std::move(value)) {} 
    std::any accept(ASTVisitor& visitor) override { return visitor.visitSetExpr(*this); }
};

class ThisExpr : public Expr {
public:
    Token keyword;
    explicit ThisExpr(Token keyword) : keyword(keyword) {} 
    std::any accept(ASTVisitor& visitor) override { return visitor.visitThisExpr(*this); }
};

class SuperExpr : public Expr {
public:
    Token keyword;
    Token method;
    SuperExpr(Token keyword, Token method) : keyword(keyword), method(method) {} 
    std::any accept(ASTVisitor& visitor) override { return visitor.visitSuperExpr(*this); }
};

class VarDecl : public Stmt {
public:
    Token name;
    std::unique_ptr<Type> type;
    std::unique_ptr<Expr> initializer;
    bool isMutable;
    VarDecl(Token name, std::unique_ptr<Type> type, std::unique_ptr<Expr> initializer, bool isMutable)
        : name(name), type(std::move(type)), initializer(std::move(initializer)), isMutable(isMutable) {} 
    std::any accept(ASTVisitor& visitor) override { return visitor.visitVarDecl(*this); }
};

class FunctionDecl : public Stmt {
public:
    Token name;
    std::vector<std::pair<Token, std::unique_ptr<Type>>> params;
    std::unique_ptr<Type> returnType;
    std::unique_ptr<Stmt> body;
    FunctionDecl(Token name, std::vector<std::pair<Token, std::unique_ptr<Type>>> params,
                 std::unique_ptr<Type> returnType, std::unique_ptr<Stmt> body)
        : name(name), params(std::move(params)), returnType(std::move(returnType)), body(std::move(body)) {} 
    std::any accept(ASTVisitor& visitor) override { return visitor.visitFunctionDecl(*this); }
};

class ClassDecl : public Stmt {
public:
    Token name;
    std::vector<std::unique_ptr<Type>> superclasses;
    std::vector<std::unique_ptr<Stmt>> methods;
    ClassDecl(Token name, std::vector<std::unique_ptr<Type>> superclasses,
              std::vector<std::unique_ptr<Stmt>> methods)
        : name(name), superclasses(std::move(superclasses)), methods(std::move(methods)) {} 
    std::any accept(ASTVisitor& visitor) override { return visitor.visitClassDecl(*this); }
};

class ExpressionStmt : public Stmt {
public:
    std::unique_ptr<Expr> expression;
    explicit ExpressionStmt(std::unique_ptr<Expr> expression)
        : expression(std::move(expression)) {} 
    std::any accept(ASTVisitor& visitor) override { return visitor.visitExpressionStmt(*this); }
};

class IfStmt : public Stmt {
public:
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> thenBranch;
    std::unique_ptr<Stmt> elseBranch;
    IfStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> thenBranch,
           std::unique_ptr<Stmt> elseBranch)
        : condition(std::move(condition)), thenBranch(std::move(thenBranch)), 
          elseBranch(std::move(elseBranch)) {} 
    std::any accept(ASTVisitor& visitor) override { return visitor.visitIfStmt(*this); }
};

class BlockStmt : public Stmt {
public:
    std::vector<std::unique_ptr<Stmt>> statements;
    explicit BlockStmt(std::vector<std::unique_ptr<Stmt>> statements)
        : statements(std::move(statements)) {} 
    std::any accept(ASTVisitor& visitor) override { return visitor.visitBlockStmt(*this); }
};

class WhileStmt : public Stmt {
public:
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> body;
    WhileStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> body)
        : condition(std::move(condition)), body(std::move(body)) {} 
    std::any accept(ASTVisitor& visitor) override { return visitor.visitWhileStmt(*this); }
};

class ReturnStmt : public Stmt {
public:
    Token keyword;
    std::unique_ptr<Expr> value;
    ReturnStmt(Token keyword, std::unique_ptr<Expr> value)
        : keyword(keyword), value(std::move(value)) {} 
    std::any accept(ASTVisitor& visitor) override { return visitor.visitReturnStmt(*this); }
};