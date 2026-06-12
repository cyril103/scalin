// src/codegen/codegen.hpp
#pragma once
#include "ast.hpp"
#include <string>
#include <sstream>

class CodeGenerator : public ASTVisitor {
public:
    std::string generate(std::unique_ptr<Stmt>& stmt);
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
    std::ostringstream output;
    std::vector<std::string> dataSection;
    int labelCounter = 0;
    void emit(const std::string& instruction);
    void emit(const std::string& instruction, const std::string& operand);
    void loadConstant(const std::any& value);
    std::string newLabel();
};