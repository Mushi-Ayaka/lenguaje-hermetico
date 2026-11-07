#pragma once
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <optional>
#include <utility>
#include <variant>

// ============================================================================
// AST del Lenguaje Hermético
// ============================================================================

namespace herm {

// --- Forward declarations ---
struct AstSdf;

// --- Expression node ---
struct ExprNode {
    std::string text; // textual expression (parsed as string, resolved later)
    int line = 0;     // linea de origen, solo para diagnostico (no se serializa)
};

// --- Colors ---
struct Color3 {
    ExprNode r, g, b;
};

struct Vec3Expr {
    ExprNode x, y, z;
};

// --- Transform ---
struct AstTransform {
    Vec3Expr translate;
    Vec3Expr rotate;
    Vec3Expr scale;
};

// --- SDF primitive ---
enum class SdfPrimType {
    SPHERE, BOX, CYLINDER, CAPSULE, TORUS, PLANE,
    CONE, ROUNDED_BOX, OCTAHEDRON, TESSERACT, SPHERE4D, CUSTOM
};

struct SdfPrimitive {
    SdfPrimType type;
    std::vector<ExprNode> params;
};

// --- SDF operator ---
enum class SdfOpType {
    UNION, SUBTRACT, INTERSECT,
    SMOOTH_UNION, SMOOTH_SUBTRACT, SMOOTH_INTERSECT,
    REPEAT, TWIST, ELONGATE, DISPLACE
};

struct SdfOperator {
    SdfOpType type;
    std::unique_ptr<AstSdf> left;
    std::unique_ptr<AstSdf> right;  // null for unary ops
    std::optional<ExprNode> k;      // smoothing factor
    std::optional<ExprNode> param;  // spacing, angle, size, or displace function
};

struct AstSdf {
    std::variant<SdfPrimitive, SdfOperator> data;
};

// --- Let ---
struct AstLet {
    std::string name;
    ExprNode value;
};

// --- Fn (pure mathematical expression function) ---
struct AstFn {
    std::string name;
    std::vector<std::string> params;  // parameter names
    std::vector<std::pair<std::string, std::string>> body_lets; // DL-14: internal lets (name, raw value text)
    std::string body_expr;            // raw expression text (final return expression)
};

// --- Light ---
struct AstLight {
    std::string name;
    std::optional<std::string> type;        // directional | point
    std::optional<Vec3Expr> direction;
    std::optional<Vec3Expr> position;
    std::optional<Color3>   color;
    std::optional<ExprNode> intensity;
    std::optional<ExprNode> falloff;
};

// --- Material ---
struct AstMaterial {
    std::string name;
    std::optional<std::vector<ExprNode>> tensor; // 8 expressions
    std::optional<std::string> blend_mode; // alpha | add | replace
    std::optional<Color3> base_color; // [r, g, b]
    std::optional<ExprNode> roughness;
    std::optional<ExprNode> metallic;
    std::optional<Color3> emission; // [r, g, b]
    std::optional<ExprNode> opacity;
};

// --- Node body ---
struct AstNode {
    std::string name;
    std::optional<std::string> mode; // solid | volume
    std::unique_ptr<AstSdf> sdf;
    std::optional<std::string> material; // material name
    std::optional<AstTransform> transform;
    std::optional<ExprNode> density_scale;
    std::optional<std::string> blend_mode;
    std::vector<std::unique_ptr<AstNode>> children; // for groups
    std::optional<std::string> def_ref; // for instance: name of def
    std::vector<ExprNode> args; // instance arguments for param'd defs
    std::optional<AstTransform> instance_transform; // transform for instance
};

// --- Def (reusable definition) ---
struct AstDef {
    std::string name;
    std::vector<std::string> params; // parameter names for param'd defs
    std::unique_ptr<AstNode> node;
};

// --- Camera ---
struct AstCamera {
    std::optional<Vec3Expr> position;
    std::optional<Vec3Expr> target;
    std::optional<Vec3Expr> up;
    std::optional<ExprNode> fov;
    std::optional<ExprNode> aperture;
    std::optional<ExprNode> focal_dist;
};

// --- Scene ---
struct AstScene {
    std::optional<ExprNode> width;  // X
    std::optional<ExprNode> height; // Y
    std::optional<ExprNode> w_frames; // W
    std::optional<AstCamera> camera;
    std::optional<Color3> background;
};

// --- Program (root) ---
struct AstProgram {
    std::optional<AstScene> scene;
    std::vector<AstLet> lets;
    std::vector<AstFn> fns;
    std::vector<AstLight> lights;
    std::vector<AstMaterial> materials;
    std::vector<AstDef> defs;
    std::vector<std::unique_ptr<AstNode>> nodes;
};

} // namespace herm
