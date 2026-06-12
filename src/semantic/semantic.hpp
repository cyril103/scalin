// src/semantic/semantic.hpp
#pragma once
#include "ast.hpp"
#include <unordered_map>
#include <vector>
#include <string>

class SemanticAnalyzer : public ASTVisitor {
public:
    void analyze(std::unique_ptr<Stmt>& stmt);
    std::any visitLiteralExpr(Expr& expr) override;
    std::any visitBinaryExpr(Expr& expr) override;
    std::any visitUnaryExpr(Expr& expr) override;
    std::any visitVariableExpr(Expr& expr) override;
    std::any visitGroupingExpr(Expr& expr) override;
    std::any visitCallExpr(Expr& expr) override;
    std::any visitGetExpr(Expr& expr) override;
    std::any visitSetExpr(Expr& expr) override;
    std::any visitThisExpr(Expr& expr) override;
    std::any visitSuperExpr(Expr& expr) override;
    std::any visitVarDecl(Stmt& stmt) override;
    std::any visitFunctionDecl(Stmt& stmt) override;
    std::any visitClassDecl(Stmt& stmt) override;
    std::any visitExpressionStmt(Stmt& stmt) override;
    std::any visitIfStmt(Stmt& stmt) override;
    std::any visitBlockStmt(Stmt& stmt) override;
    std::any visitWhileStmt(Stmt& stmt) override;
    std::any visitReturnStmt(Stmt& stmt) override;

private:
    struct Environment {
        std::unordered_map<std::string, std::unique_ptr<Type>> variables;
        Environment* parent = nullptr;
    };
    Environment* currentEnv = nullptr;
    std::vector<std::string> errors;
    void pushEnvironment();
    void popEnvironment();
    void declareVariable(const std::string& name, std::unique_ptr<Type> type);
    std::unique_ptr<Type> getVariableType(const std::string& name);
    void addError(const std::string& message);
};