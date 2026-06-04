#pragma once
#include "herm_ast.h"
#include "herm_lexer.h"
#include <vector>
#include <memory>
#include <functional>
#include <stdexcept>

namespace herm {

class ParseError : public std::runtime_error {
public:
    explicit ParseError(const std::string& msg) : std::runtime_error(msg) {}
};

using FileLoader = std::function<std::string(const std::string& path)>;

class Parser {
public:
    explicit Parser(std::vector<Token> tokens, FileLoader loader = nullptr);
    AstProgram parse();

private:
    Token peek() const;
    Token advance();
    Token expect(TokenType type);
    bool skip(TokenType type);
    // Top-level
    void parse_import(AstProgram& prog);
    AstScene parse_scene();
    AstCamera parse_camera();
    AstLet parse_let();
    AstFn parse_fn();
    AstLight parse_light();
    AstMaterial parse_material();
    AstDef parse_def();
    std::unique_ptr<AstNode> parse_node();
    std::unique_ptr<AstNode> parse_instance();
    void parse_node_body(AstNode& node);

    // SDF
    std::unique_ptr<AstSdf> parse_sdf();

    // Expressions
    ExprNode parse_expr();

    // Colors and vectors
    Color3 parse_color3();
    Vec3Expr parse_vec3();

    // Helpers
    std::string parse_string_or_ident();

    std::vector<Token> tokens;
    size_t pos;
    FileLoader loader;
};

} // namespace herm
