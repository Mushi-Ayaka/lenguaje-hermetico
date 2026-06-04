#pragma once
#include "../contrato/rih.h"
#include "herm_ast.h"
#include <unordered_map>
#include <stdexcept>

namespace herm {

class ResolverError : public std::runtime_error {
public:
    explicit ResolverError(const std::string& msg) : std::runtime_error(msg) {}
};

class Resolver {
public:
    Resolver(const AstProgram& ast);
    Rih resolve();

private:
    // Scene
    void resolveScene(const AstScene& ast);
    void resolveCamera(const AstCamera& ast);
    f32 evalExpr(const ExprNode& expr) const;
    Vec3 evalVec3(const Vec3Expr& vec) const;
    ColorRGB evalColor3(const Color3& col) const;
    Expr toExpr(const ExprNode& node, f32 default_val = 0.0f) const;

    // Lights
    void resolveLight(const AstLight& ast);

    // Materials
    u32 resolveMaterial(const AstMaterial& ast);

    // SDF — flattens tree into list, returns root index
    u32 resolveSdfTree(const AstSdf& ast, std::vector<SdfNode>& sdf_list);

    // Nodes
    u32 resolveNode(const AstNode& ast, std::vector<Node>& node_list);
    void resolveDefs();

    // SDF substitution (for param'd defs)
    std::unique_ptr<AstSdf> cloneSubstSdf(const AstSdf& src,
        const std::unordered_map<std::string, std::string>& subst) const;
    std::string substExprText(const std::string& text,
        const std::unordered_map<std::string, std::string>& subst) const;
    std::string expandExpression(const std::string& text, int depth = 0) const;

    // State
    const AstProgram& ast;
    Rih rih;
    u32 next_id = 0;
    std::unordered_map<std::string, u32> material_map;
    std::unordered_map<std::string, u32> def_map;
    std::unordered_map<std::string, const AstDef*> ast_def_map; // param'd defs
    std::unordered_map<std::string, std::string> let_table;     // let variables
    std::unordered_map<std::string, const AstFn*> fn_table;     // fn pure functions
};

} // namespace herm
