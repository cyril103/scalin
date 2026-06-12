// src/semantic/semantic.cpp
#include "semantic.hpp"
#include <sstream>

void SemanticAnalyzer::analyze(std::unique_ptr<Stmt>& stmt) {
    currentEnv = new Environment();
    currentEnv->parent = nullptr;
    stmt->accept(*this);
    while (currentEnv != nullptr) {
        Environment* parent = currentEnv->parent;
        delete currentEnv;
        currentEnv = parent;
    }
    if (!errors.empty()) {
        std::ostringstream oss;
        for (const auto& error : errors) oss << error << "\n";
        throw std::runtime_error(oss.str());
    }
}

void SemanticAnalyzer::pushEnvironment() {
    currentEnv = new Environment{currentEnv, {}};
}

void SemanticAnalyzer::popEnvironment() {
    Environment* parent = currentEnv->parent;
    delete currentEnv;
    currentEnv = parent;
}

void SemanticAnalyzer::declareVariable(const std::string& name, std::unique_ptr<Type> type) {
    if (currentEnv->variables.find(name) != currentEnv->variables.end()) {
        addError("Variable '" + name + "' déjà déclarée.");
    }
    currentEnv->variables[name] = std::move(type);
}

std::unique_ptr<Type> SemanticAnalyzer::getVariableType(const std::string& name) {
    for (Environment* env = currentEnv; env != nullptr; env = env->parent) {
        auto it = env->variables.find(name);
        if (it != env->variables.end()) {
            return it->second->clone();
        }
    }
    addError("Variable '" + name + "' non trouvée.");
    return std::make_unique<PrimitiveType>(TokenType::UNIT);
}

void SemanticAnalyzer::addError(const std::string& message) {
    errors.push_back(message);
}

std::any SemanticAnalyzer::visitLiteralExpr(Expr&) { return nullptr; }
std::any SemanticAnalyzer::visitBinaryExpr(Expr& expr) {
    auto& e = dynamic_cast<BinaryExpr&>(expr);
    e.left->accept(*this); e.right->accept(*this);
    return nullptr;
}
std::any SemanticAnalyzer::visitUnaryExpr(Expr& expr) {
    auto& e = dynamic_cast<UnaryExpr&>(expr);
    e.right->accept(*this);
    return nullptr;
}
std::any SemanticAnalyzer::visitVariableExpr(Expr& expr) {
    auto& e = dynamic_cast<VariableExpr&>(expr);
    getVariableType(e.name.lexeme);
    return nullptr;
}
std::any SemanticAnalyzer::visitGroupingExpr(Expr& expr) {
    auto& e = dynamic_cast<GroupingExpr&>(expr);
    e.expression->accept(*this);
    return nullptr;
}
std::any SemanticAnalyzer::visitCallExpr(Expr& expr) {
    auto& e = dynamic_cast<CallExpr&>(expr);
    e.callee->accept(*this);
    for (auto& arg : e.arguments) arg->accept(*this);
    return nullptr;
}
std::any SemanticAnalyzer::visitGetExpr(Expr&) { return nullptr; }
std::any SemanticAnalyzer::visitSetExpr(Expr& expr) {
    auto& e = dynamic_cast<SetExpr&>(expr);
    e.object->accept(*this); e.value->accept(*this);
    return nullptr;
}
std::any SemanticAnalyzer::visitThisExpr(Expr&) { return nullptr; }
std::any SemanticAnalyzer::visitSuperExpr(Expr&) { return nullptr; }

std::any SemanticAnalyzer::visitVarDecl(Stmt& stmt) {
    auto& s = dynamic_cast<VarDecl&>(stmt);
    if (s.initializer) s.initializer->accept(*this);
    if (!s.type) {
        if (s.initializer) s.type = std::make_unique<PrimitiveType>(TokenType::INT);
        else addError("Type requis pour '" + s.name.lexeme + "' sans initialisation.");
    }
    declareVariable(s.name.lexeme, s.type ? s.type->clone() : std::make_unique<PrimitiveType>(TokenType::UNIT));
    return nullptr;
}

std::any SemanticAnalyzer::visitFunctionDecl(Stmt& stmt) {
    auto& s = dynamic_cast<FunctionDecl&>(stmt);
    pushEnvironment();
    for (auto& param : s.params) {
        declareVariable(param.first.lexeme, param.second->clone());
    }
    s.body->accept(*this);
    popEnvironment();
    return nullptr;
}

std::any SemanticAnalyzer::visitClassDecl(Stmt& stmt) {
    auto& s = dynamic_cast<ClassDecl&>(stmt);
    pushEnvironment();
    for (auto& method : s.methods) method->accept(*this);
    popEnvironment();
    return nullptr;
}

std::any SemanticAnalyzer::visitExpressionStmt(Stmt& stmt) {
    auto& s = dynamic_cast<ExpressionStmt&>(stmt);
    s.expression->accept(*this);
    return nullptr;
}

std::any SemanticAnalyzer::visitIfStmt(Stmt& stmt) {
    auto& s = dynamic_cast<IfStmt&>(stmt);
    s.condition->accept(*this);
    s.thenBranch->accept(*this);
    if (s.elseBranch) s.elseBranch->accept(*this);
    return nullptr;
}

std::any SemanticAnalyzer::visitBlockStmt(Stmt& stmt) {
    auto& s = dynamic_cast<BlockStmt&>(stmt);
    pushEnvironment();
    for (auto& statement : s.statements) statement->accept(*this);
    popEnvironment();
    return nullptr;
}

std::any SemanticAnalyzer::visitWhileStmt(Stmt& stmt) {
    auto& s = dynamic_cast<WhileStmt&>(stmt);
    s.condition->accept(*this);
    s.body->accept(*this);
    return nullptr;
}

std::any SemanticAnalyzer::visitReturnStmt(Stmt& stmt) {
    auto& s = dynamic_cast<ReturnStmt&>(stmt);
    if (s.value) s.value->accept(*this);
    return nullptr;
}