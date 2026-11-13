#include <stdexcept>
#include <cmath>
#include <cstdio>
#include "herm_resolver.h"
#include "herm_eval.h"

namespace herm {

namespace {

// DL-09: limite duro de expansion. Sin CSE, una `fn` que recibe como argumento
// otra `fn` grande se duplica en cada uso del parametro (domain_warp2d supera
// los 300 MB). Preferimos un fallo ruidoso a materializar el arbol.
constexpr size_t kMaxExpansionBytes = 16u * 1024u * 1024u;
constexpr size_t kMaxCacheEntryBytes = 1u * 1024u * 1024u;
constexpr size_t kMaxCacheBytes = 64u * 1024u * 1024u;

// Formatea una constante f32 con precision suficiente para round-trip.
std::string formatConst(f32 v) {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.9g", (double)v);
    return std::string(buf);
}

bool isIdentifierChar(char c) {
    return std::isalnum((unsigned char)c) || c == '_';
}

// Nombres que evalExpression resuelve contra el contexto de runtime. Una
// expresion que use cualquiera de ellos NO se pliega a constante aunque su
// valor coincida en dos muestras: el .rih debe conservar la dependencia.
bool usesRuntimeVar(const std::string& t) {
    static const char* vars[] = {
        "x", "y", "z", "w", "t",
        "nx", "ny", "nz", "vx", "vy", "vz", "lx", "ly", "lz"
    };
    size_t i = 0;
    while (i < t.size()) {
        if (std::isalpha((unsigned char)t[i]) || t[i] == '_') {
            size_t start = i;
            while (i < t.size() && isIdentifierChar(t[i])) i++;
            std::string name = t.substr(start, i - start);
            for (const char* v : vars) {
                if (name == v) return true;
            }
        } else {
            i++;
        }
    }
    return false;
}

} // namespace

Resolver::Resolver(const AstProgram& ast) : ast(ast) {}

Rih Resolver::resolve() {
    rih = Rih{};
    rih.version = 1;
    next_id = 0;

    if (ast.scene) resolveScene(*ast.scene);

    for (const auto& f : ast.fns) {
        fn_table[f.name] = &f;
    }

    // Collect and resolve let variables (textual substitution)
    for (auto& lt : ast.lets) {
        let_table[lt.name] = lt.value.text;
    }
    // Resolve chains: iterate until stable
    for (size_t i = 0; i < let_table.size(); i++) {
        bool changed = false;
        for (auto& [name, val] : let_table) {
            std::string new_val = expandExpression(val);
            // DL-09 A: si el valor ya no depende de nada, guardarlo como
            // literal. Evita arrastrar el arbol expandido por cada uso.
            f32 folded = 0.0f;
            if (tryFoldConstant(new_val, folded)) new_val = formatConst(folded);
            if (new_val != val) { val = new_val; changed = true; }
        }
        if (!changed) break;
    }

    for (auto& l : ast.lights) {
        resolveLight(l);
    }

    for (auto& m : ast.materials) {
        resolveMaterial(m);
    }

    resolveDefs();

    for (auto& n : ast.nodes) {
        resolveNode(*n, rih.nodes);
    }

    return rih;
}

// ============================================================================
// Scene
// ============================================================================

void Resolver::resolveScene(const AstScene& ast) {
    if (ast.width) rih.scene.width = (u32)evalExpr(*ast.width);
    if (ast.height) rih.scene.height = (u32)evalExpr(*ast.height);
    if (ast.w_frames) {
        rih.scene.w_frames = (u32)evalExpr(*ast.w_frames);
        rih.scene.w_max = (f32)(rih.scene.w_frames - 1);
    }
    if (ast.camera) resolveCamera(*ast.camera);
    if (ast.background) rih.scene.background = evalColor3(*ast.background);
}

void Resolver::resolveCamera(const AstCamera& ast) {
    if (ast.position) {
        auto v = evalVec3(*ast.position);
        rih.scene.camera.position = v;
    }
    if (ast.target) {
        auto v = evalVec3(*ast.target);
        rih.scene.camera.target = v;
    }
    if (ast.up) {
        auto v = evalVec3(*ast.up);
        rih.scene.camera.up = v;
    }
    if (ast.fov) rih.scene.camera.fov = evalExpr(*ast.fov);
    if (ast.aperture) rih.scene.camera.aperture = evalExpr(*ast.aperture);
    if (ast.focal_dist) rih.scene.camera.focal_dist = evalExpr(*ast.focal_dist);
}

f32 Resolver::evalExpr(const ExprNode& expr) const {
    if (expr.text.empty()) return 0.0f;
    std::string text = expandExpression(expr.text);

    // DL-09 A: plegado constante real.
    f32 folded = 0.0f;
    if (tryFoldConstant(text, folded)) return folded;

    // No es un literal: evaluar con el mismo evaluador que usa el runtime
    // (DL-29). Antes se devolvia 0.0f en silencio.
    float a = evalExpression(text, 0.0f, 0.0f, 0.0f, 0.0f);
    float b = evalExpression(text, 1.0f, 2.0f, 3.0f, 7.0f);
    // Un campo escalar de escena se pliega a constante en el .rih: si el valor
    // cambia al cambiar el contexto espacial/temporal, no es una constante.
    if (!(a == b)) {
        throw ResolverError("la expresion '" + text + "' depende de x/y/z/w y se usa "
                            "en un campo escalar constante");
    }
    // El .rih es JSON: 'inf'/'nan' lo rompen. Un campo escalar debe ser finito.
    if (!std::isfinite(a)) {
        throw ResolverError("la expresion '" + text + "' no da un numero finito "
                            "(inf/nan) y se usa en un campo escalar");
    }
    return a;
}

Vec3 Resolver::evalVec3(const Vec3Expr& vec) const {
    return {evalExpr(vec.x), evalExpr(vec.y), evalExpr(vec.z)};
}

ColorRGB Resolver::evalColor3(const Color3& col) const {
    return {evalExpr(col.r), evalExpr(col.g), evalExpr(col.b)};
}

Expr Resolver::toExpr(const ExprNode& node, f32 default_val) const {
    Expr e;
    e.line = node.line;
    if (node.text.empty()) {
        e.is_expr = false;
        e.constant = default_val;
        return e;
    }
    std::string text = expandExpression(node.text);

    // DL-09 A: si la expresion es constante, guardarla como numero en vez de
    // arrastrar el texto expandido hasta el .rih.
    f32 folded = 0.0f;
    if (tryFoldConstant(text, folded)) {
        e.is_expr = false;
        e.constant = folded;
        return e;
    }
    e.is_expr = true;
    e.expression = text;
    return e;
}

// ============================================================================
// Materials
// ============================================================================

u32 Resolver::resolveMaterial(const AstMaterial& ast) {
    Material mat;
    mat.id = (u32)rih.materials.size();
    mat.name = ast.name;

    for (size_t i = 0; i < 8; i++) {
        if (ast.tensor && i < ast.tensor->size()) {
            mat.tensor[i] = toExpr((*ast.tensor)[i], 0.0f);
        } else {
            // Default alpha (tensor[7]) = 1.0: an unset opacity must mean
            // opaque, otherwise the RI optimizer culls every node as "dead".
            mat.tensor[i] = toExpr(ExprNode{}, i == 7 ? 1.0f : 0.0f);
        }
    }

    if (ast.blend_mode) {
        if (*ast.blend_mode == "add" || *ast.blend_mode == "sumar")
            mat.blend_mode = BlendMode::ADD;
        else if (*ast.blend_mode == "replace" || *ast.blend_mode == "reemplazar")
            mat.blend_mode = BlendMode::REPLACE;
        else
            mat.blend_mode = BlendMode::ALPHA;
    }

    if (ast.base_color) {
        auto c = evalColor3(*ast.base_color);
        auto mkConst = [](f32 v) { Expr e; e.is_expr = false; e.constant = v; return e; };
        mat.tensor[4] = mkConst(c.r);
        mat.tensor[5] = mkConst(c.g);
        mat.tensor[6] = mkConst(c.b);
    }

    if (ast.roughness) mat.roughness = evalExpr(*ast.roughness);
    if (ast.metallic)  mat.metallic  = evalExpr(*ast.metallic);
    if (ast.emission) {
        auto e = evalColor3(*ast.emission);
        mat.emission = e;
    }
    if (ast.opacity) {
        float o = evalExpr(*ast.opacity);
        Expr e; e.is_expr = false; e.constant = o;
        mat.tensor[7] = e;
    }

    rih.materials.push_back(mat);
    material_map[ast.name] = mat.id;
    return mat.id;
}

// ============================================================================
// Lights
// ============================================================================

void Resolver::resolveLight(const AstLight& ast) {
    Light L;
    L.id = (u32)rih.lights.size();
    L.name = ast.name;

    if (ast.type) {
        auto& t = *ast.type;
        if (t == "point" || t == "puntual")
            L.type = LightType::POINT;
        else
            L.type = LightType::DIRECTIONAL;
    }

    if (ast.direction) {
        auto v = evalVec3(*ast.direction);
        L.direction = v;
    }
    if (ast.position) {
        auto v = evalVec3(*ast.position);
        L.position = v;
    }
    if (ast.color) {
        auto c = evalColor3(*ast.color);
        L.color = c;
    }
    if (ast.intensity) L.intensity = evalExpr(*ast.intensity);
    if (ast.falloff)   L.falloff   = evalExpr(*ast.falloff);

    rih.lights.push_back(L);
}

// ============================================================================
// SDF tree flattening
// ============================================================================

static SdfType mapPrimType(SdfPrimType t) {
    switch (t) {
        case SdfPrimType::SPHERE:  return SdfType::SPHERE;
        case SdfPrimType::BOX:     return SdfType::BOX;
        case SdfPrimType::CYLINDER: return SdfType::CYLINDER;
        case SdfPrimType::CAPSULE: return SdfType::CAPSULE;
        case SdfPrimType::TORUS:   return SdfType::TORUS;
        case SdfPrimType::PLANE:   return SdfType::PLANE;
        case SdfPrimType::CONE:    return SdfType::CONE;
        case SdfPrimType::ROUNDED_BOX: return SdfType::ROUNDED_BOX;
        case SdfPrimType::OCTAHEDRON: return SdfType::OCTAHEDRON;
        case SdfPrimType::TESSERACT: return SdfType::TESSERACT;
        case SdfPrimType::SPHERE4D: return SdfType::SPHERE4D;
        case SdfPrimType::CUSTOM:   return SdfType::CUSTOM;
    }
    return SdfType::SPHERE;
}

static SdfType mapOpType(SdfOpType t) {
    switch (t) {
        case SdfOpType::UNION:   return SdfType::UNION;
        case SdfOpType::SUBTRACT: return SdfType::SUBTRACT;
        case SdfOpType::INTERSECT: return SdfType::INTERSECT;
        case SdfOpType::SMOOTH_UNION: return SdfType::SMOOTH_UNION;
        case SdfOpType::SMOOTH_SUBTRACT: return SdfType::SMOOTH_SUBTRACT;
        case SdfOpType::SMOOTH_INTERSECT: return SdfType::SMOOTH_INTERSECT;
        case SdfOpType::REPEAT:   return SdfType::REPEAT;
        case SdfOpType::TWIST:    return SdfType::TWIST;
        case SdfOpType::ELONGATE: return SdfType::ELONGATE;
        case SdfOpType::DISPLACE: return SdfType::DISPLACE;
    }
    return SdfType::UNION;
}

u32 Resolver::resolveSdfTree(const AstSdf& ast_sdf, std::vector<SdfNode>& sdf_list) {
    SdfNode node;
    u32 idx = (u32)sdf_list.size();

    // Reserve this node's slot FIRST: the contract (rih.h) is that the root of
    // each node's flat sdf list is at index 0. Children are resolved after the
    // placeholder, so they get higher indices and child_a/child_b stay valid
    // even after reallocations (node is written back by index at the end).
    sdf_list.push_back(SdfNode{});

    if (std::holds_alternative<SdfPrimitive>(ast_sdf.data)) {
        auto& prim = std::get<SdfPrimitive>(ast_sdf.data);
        node.type = mapPrimType(prim.type);
        for (size_t i = 0; i < 4; i++) {
            if (i < prim.params.size())
                node.params[i] = toExpr(prim.params[i]);
            else
                node.params[i] = toExpr(ExprNode{}, 0.0f);
        }
    } else if (std::holds_alternative<SdfOperator>(ast_sdf.data)) {
        auto& op = std::get<SdfOperator>(ast_sdf.data);
        node.type = mapOpType(op.type);

        if (op.left) node.child_a = resolveSdfTree(*op.left, sdf_list);
        if (op.right) node.child_b = resolveSdfTree(*op.right, sdf_list);

        if (op.k) node.k = evalExpr(*op.k);
        if (op.param) {
            if (node.type == SdfType::DISPLACE)
                node.displace_func = op.param->text;
            else
                node.spacing = evalExpr(*op.param);
        }
    }

    sdf_list[idx] = node;
    return idx;
}

// ============================================================================
// Nodes
// ============================================================================

u32 Resolver::resolveNode(const AstNode& ast_node, std::vector<Node>& node_list) {
    Node n;
    n.id = next_id++;

    if (ast_node.def_ref.has_value()) {
        auto def_name = ast_node.def_ref.value();

        // Check if this is a parameterized def
        auto pit = ast_def_map.find(def_name);
        if (pit != ast_def_map.end() && !ast_node.args.empty()) {
            // Parametrized instance — substitute and resolve inline as SDF node
            auto& def = *pit->second;
            if (def.params.size() != ast_node.args.size()) {
                throw ResolverError("def '" + def_name + "' espera " +
                    std::to_string(def.params.size()) + " argumentos pero se recibieron " +
                    std::to_string(ast_node.args.size()));
            }

            // Build substitution map
            std::unordered_map<std::string, std::string> subst;
            for (size_t i = 0; i < def.params.size(); i++) {
                subst[def.params[i]] = ast_node.args[i].text;
            }

            n.type = NodeType::SDF;
            n.name = def_name;

            // Substitute and resolve SDF tree
            if (def.node->sdf) {
                auto subst_sdf = cloneSubstSdf(*def.node->sdf, subst);
                resolveSdfTree(*subst_sdf, n.sdf_nodes);
            }

            // Copy mode from def (with substitution)
            if (def.node->mode) {
                n.mode = (*def.node->mode == "volume" || *def.node->mode == "volumen")
                    ? NodeMode::VOLUME : NodeMode::SOLID;
            }

            // Copy material from def
            if (def.node->material) {
                auto mit = material_map.find(*def.node->material);
                if (mit == material_map.end())
                    throw ResolverError("material '" + *def.node->material + "' no definido");
                n.material_id = mit->second;
            }

            if (def.node->density_scale)
                n.density_scale = evalExpr(*def.node->density_scale);

            if (def.node->blend_mode) {
                auto& bm = *def.node->blend_mode;
                if (bm == "add" || bm == "sumar") n.blend_mode = BlendMode::ADD;
                else if (bm == "replace" || bm == "reemplazar") n.blend_mode = BlendMode::REPLACE;
                else n.blend_mode = BlendMode::ALPHA;
            }

            // Transform: start with identity, add def's transform (with substitution),
            // then compose with instance transform
            n.transform.translate[0] = toExpr({});
            n.transform.translate[1] = toExpr({});
            n.transform.translate[2] = toExpr({});
            n.transform.rotate[0] = toExpr({});
            n.transform.rotate[1] = toExpr({});
            n.transform.rotate[2] = toExpr({});
            n.transform.scale[0] = toExpr({}, 1.0f);
            n.transform.scale[1] = toExpr({}, 1.0f);
            n.transform.scale[2] = toExpr({}, 1.0f);

            // Apply def's transform (with substitution)
            if (def.node->transform) {
                auto& dt = *def.node->transform;
                auto substT = [&](const ExprNode& e) -> ExprNode {
                    ExprNode r;
                    r.text = substExprText(e.text, subst);
                    return r;
                };
                if (!dt.translate.x.text.empty()) n.transform.translate[0] = toExpr(substT(dt.translate.x));
                if (!dt.translate.y.text.empty()) n.transform.translate[1] = toExpr(substT(dt.translate.y));
                if (!dt.translate.z.text.empty()) n.transform.translate[2] = toExpr(substT(dt.translate.z));
                if (!dt.rotate.x.text.empty()) n.transform.rotate[0] = toExpr(substT(dt.rotate.x));
                if (!dt.rotate.y.text.empty()) n.transform.rotate[1] = toExpr(substT(dt.rotate.y));
                if (!dt.rotate.z.text.empty()) n.transform.rotate[2] = toExpr(substT(dt.rotate.z));
                if (!dt.scale.x.text.empty()) n.transform.scale[0] = toExpr(substT(dt.scale.x), 1.0f);
                if (!dt.scale.y.text.empty()) n.transform.scale[1] = toExpr(substT(dt.scale.y), 1.0f);
                if (!dt.scale.z.text.empty()) n.transform.scale[2] = toExpr(substT(dt.scale.z), 1.0f);
            }

            // Apply instance transform on top
            if (ast_node.instance_transform) {
                auto& t = *ast_node.instance_transform;
                if (!t.translate.x.text.empty()) n.transform.translate[0] = toExpr(t.translate.x);
                if (!t.translate.y.text.empty()) n.transform.translate[1] = toExpr(t.translate.y);
                if (!t.translate.z.text.empty()) n.transform.translate[2] = toExpr(t.translate.z);
                if (!t.rotate.x.text.empty()) n.transform.rotate[0] = toExpr(t.rotate.x);
                if (!t.rotate.y.text.empty()) n.transform.rotate[1] = toExpr(t.rotate.y);
                if (!t.rotate.z.text.empty()) n.transform.rotate[2] = toExpr(t.rotate.z);
                if (!t.scale.x.text.empty()) n.transform.scale[0] = toExpr(t.scale.x, 1.0f);
                if (!t.scale.y.text.empty()) n.transform.scale[1] = toExpr(t.scale.y, 1.0f);
                if (!t.scale.z.text.empty()) n.transform.scale[2] = toExpr(t.scale.z, 1.0f);
            }
        } else {
            // Regular (non-parametrized) instance
            n.type = NodeType::INSTANCE;
            n.name = def_name;
            auto it = def_map.find(def_name);
            if (it == def_map.end())
                throw ResolverError("def '" + def_name + "' no definido");
            n.def_id = it->second;

            // Default identity transform
            n.transform.translate[0] = toExpr({});
            n.transform.translate[1] = toExpr({});
            n.transform.translate[2] = toExpr({});
            n.transform.rotate[0] = toExpr({});
            n.transform.rotate[1] = toExpr({});
            n.transform.rotate[2] = toExpr({});
            n.transform.scale[0] = toExpr({}, 1.0f);
            n.transform.scale[1] = toExpr({}, 1.0f);
            n.transform.scale[2] = toExpr({}, 1.0f);

            if (ast_node.instance_transform) {
                auto& t = *ast_node.instance_transform;
                if (!t.translate.x.text.empty()) n.transform.translate[0] = toExpr(t.translate.x);
                if (!t.translate.y.text.empty()) n.transform.translate[1] = toExpr(t.translate.y);
                if (!t.translate.z.text.empty()) n.transform.translate[2] = toExpr(t.translate.z);
                if (!t.rotate.x.text.empty()) n.transform.rotate[0] = toExpr(t.rotate.x);
                if (!t.rotate.y.text.empty()) n.transform.rotate[1] = toExpr(t.rotate.y);
                if (!t.rotate.z.text.empty()) n.transform.rotate[2] = toExpr(t.rotate.z);
                if (!t.scale.x.text.empty()) n.transform.scale[0] = toExpr(t.scale.x, 1.0f);
                if (!t.scale.y.text.empty()) n.transform.scale[1] = toExpr(t.scale.y, 1.0f);
                if (!t.scale.z.text.empty()) n.transform.scale[2] = toExpr(t.scale.z, 1.0f);
            }
        }
    } else if (!ast_node.children.empty()) {
        // Group node
        n.type = NodeType::GROUP;
        n.name = ast_node.name;
        n.transform.translate[0] = toExpr({});
        n.transform.translate[1] = toExpr({});
        n.transform.translate[2] = toExpr({});
        n.transform.rotate[0] = toExpr({});
        n.transform.rotate[1] = toExpr({});
        n.transform.rotate[2] = toExpr({});
        n.transform.scale[0] = toExpr({}, 1.0f);
        n.transform.scale[1] = toExpr({}, 1.0f);
        n.transform.scale[2] = toExpr({}, 1.0f);
        for (auto& child : ast_node.children) {
            n.children.push_back(resolveNode(*child, node_list));
        }
    } else {
        // SDF leaf node
        n.type = NodeType::SDF;
        n.name = ast_node.name;

        if (ast_node.sdf) {
            resolveSdfTree(*ast_node.sdf, n.sdf_nodes);
        }

        if (ast_node.mode) {
            n.mode = (*ast_node.mode == "volume" || *ast_node.mode == "volumen")
                ? NodeMode::VOLUME : NodeMode::SOLID;
        }

        if (ast_node.material) {
            auto it = material_map.find(*ast_node.material);
            if (it == material_map.end())
                throw ResolverError("material '" + *ast_node.material + "' no definido");
            n.material_id = it->second;
        }

        if (ast_node.density_scale)
            n.density_scale = evalExpr(*ast_node.density_scale);

        if (ast_node.blend_mode) {
            auto& bm = *ast_node.blend_mode;
            if (bm == "add" || bm == "sumar") n.blend_mode = BlendMode::ADD;
            else if (bm == "replace" || bm == "reemplazar") n.blend_mode = BlendMode::REPLACE;
            else n.blend_mode = BlendMode::ALPHA;
        }

        // Default identity transform
        n.transform.translate[0] = toExpr({});
        n.transform.translate[1] = toExpr({});
        n.transform.translate[2] = toExpr({});
        n.transform.rotate[0] = toExpr({});
        n.transform.rotate[1] = toExpr({});
        n.transform.rotate[2] = toExpr({});
        n.transform.scale[0] = toExpr({}, 1.0f);
        n.transform.scale[1] = toExpr({}, 1.0f);
        n.transform.scale[2] = toExpr({}, 1.0f);

        if (ast_node.transform) {
            auto& t = *ast_node.transform;
            if (!t.translate.x.text.empty()) n.transform.translate[0] = toExpr(t.translate.x);
            if (!t.translate.y.text.empty()) n.transform.translate[1] = toExpr(t.translate.y);
            if (!t.translate.z.text.empty()) n.transform.translate[2] = toExpr(t.translate.z);
            if (!t.rotate.x.text.empty()) n.transform.rotate[0] = toExpr(t.rotate.x);
            if (!t.rotate.y.text.empty()) n.transform.rotate[1] = toExpr(t.rotate.y);
            if (!t.rotate.z.text.empty()) n.transform.rotate[2] = toExpr(t.rotate.z);
            if (!t.scale.x.text.empty()) n.transform.scale[0] = toExpr(t.scale.x, 1.0f);
            if (!t.scale.y.text.empty()) n.transform.scale[1] = toExpr(t.scale.y, 1.0f);
            if (!t.scale.z.text.empty()) n.transform.scale[2] = toExpr(t.scale.z, 1.0f);
        }
    }

    node_list.push_back(n);
    return n.id;
}

// ============================================================================
// SDF substitution (for parameterized defs)
// ============================================================================

std::string Resolver::substExprText(const std::string& text,
    const std::unordered_map<std::string, std::string>& subst) const
{
    std::string t = text;
    auto isWord = [](char c) { return std::isalnum((unsigned char)c) || c == '_'; };
    for (auto& [param, arg] : subst) {
        size_t pos = 0;
        while ((pos = t.find(param, pos)) != std::string::npos) {
            bool bl = (pos == 0 || !isWord(t[pos - 1]));
            bool br = (pos + param.size() >= t.size() || !isWord(t[pos + param.size()]));
            if (bl && br) {
                // DL-09: una sustitucion no puede materializar un arbol sin limite.
                if (t.size() - param.size() + arg.size() > kMaxExpansionBytes)
                    throw ResolverError("la sustitucion de '" + param +
                        "' supera el limite de 16 MB de expansion");
                t.replace(pos, param.size(), arg);
                pos += arg.size();
            } else {
                pos += param.size();
            }
        }
    }
    return t;
}

std::string Resolver::expandExpression(const std::string& text, int depth) const {
    if (depth > 16) return text; // Limite de recursion para evitar cuelgues
    if (text.size() > kMaxExpansionBytes)
        throw ResolverError("la expresion supera el limite de 16 MB antes de expandir");

    std::string res;
    res.reserve(text.size());
    size_t i = 0;
    while (i < text.size()) {
        if (std::isalpha((unsigned char)text[i]) || text[i] == '_') {
            size_t start = i;
            while (i < text.size() && isIdentifierChar(text[i])) i++;
            std::string name = text.substr(start, i - start);

            size_t j = i;
            while (j < text.size() && std::isspace((unsigned char)text[j])) j++;

            if (j < text.size() && text[j] == '(' && fn_table.count(name)) {
                const AstFn* fn = fn_table.at(name);
                j++; // skip '('

                std::vector<std::string> args;
                std::string current_arg;
                int parens = 0;

                while (j < text.size()) {
                    if (text[j] == '(') parens++;
                    else if (text[j] == ')') {
                        if (parens == 0) {
                            args.push_back(current_arg);
                            j++;
                            break;
                        }
                        parens--;
                    }
                    else if (text[j] == ',' && parens == 0) {
                        args.push_back(current_arg);
                        current_arg = "";
                        j++;
                        continue;
                    }
                    current_arg += text[j];
                    j++;
                }

                // DL-09 B: expandir cada argumento una sola vez antes de
                // sustituirlo. La clave de cache usa los argumentos ya
                // expandidos (y la profundidad, por el corte de recursion).
                for (auto& a : args) a = expandExpression(a, depth + 1);

                std::string key = name;
                key += '#';
                key += std::to_string(depth);
                for (auto& a : args) {
                    key += '\x1e';
                    key += a;
                }

                std::string expanded;
                auto cached = fn_expand_cache.find(key);
                if (cached != fn_expand_cache.end()) {
                    expanded = cached->second;
                } else {
                    std::unordered_map<std::string, std::string> param_subst;
                    for (size_t k = 0; k < std::min(fn->params.size(), args.size()); k++) {
                        param_subst[fn->params[k]] = "(" + args[k] + ")";
                    }

                    // DL-14: desugar internal lets — later lets and the final body
                    // see each let's name replaced by its parenthesized value
                    std::unordered_map<std::string, std::string> let_subst;
                    for (auto& [lname, lval] : fn->body_lets) {
                        std::string lv = substExprText(lval, param_subst);
                        lv = substExprText(lv, let_subst);
                        let_subst[lname] = "(" + lv + ")";
                    }

                    // body: lets primero (shadowing: un let interno pisa al param homonimo),
                    // luego params para los nombres no cubiertos por lets
                    try {
                        expanded = substExprText(fn->body_expr, let_subst);
                        expanded = substExprText(expanded, param_subst);
                        expanded = expandExpression(expanded, depth + 1);
                    } catch (const ResolverError& e) {
                        throw ResolverError("expandiendo la funcion '" + name + "': " + e.what());
                    }
                    if (expanded.size() > kMaxExpansionBytes)
                        throw ResolverError("la expansion de la funcion '" + name +
                            "' supera el limite de 16 MB de expansion");
                    if (expanded.size() <= kMaxCacheEntryBytes) {
                        if (fn_cache_bytes + expanded.size() > kMaxCacheBytes) {
                            fn_expand_cache.clear();
                            fn_cache_bytes = 0;
                        }
                        fn_cache_bytes += expanded.size();
                        fn_expand_cache.emplace(std::move(key), expanded);
                    }
                }

                if (res.size() + expanded.size() + 2 > kMaxExpansionBytes)
                    throw ResolverError("la expansion de la funcion '" + name +
                        "' supera el limite de 16 MB de expansion");
                res += "(" + expanded + ")";
                i = j;
            } else {
                res += name;
            }
        } else {
            res += text[i];
            i++;
        }
    }

    // DL-09: la sustitucion de lets va DESPUES de expandir llamadas `fn`.
    // Asi `dot3(...)` (fn de std_math_core) no se confunde con un `let dot3`
    // definido por la escena. Un let solo se sustituye como identificador
    // suelto, nunca como nombre de llamada.
    return substExprText(res, let_table);
}

bool Resolver::tryFoldConstant(const std::string& expanded, f32& out) const {
    if (expanded.empty()) return false;

    // Literal puro (camino rapido).
    try {
        size_t pos = 0;
        float v = std::stof(expanded, &pos);
        if (pos == expanded.size()) {
            if (!std::isfinite(v)) return false;
            out = v;
            return true;
        }
    } catch (...) {}

    // Nunca plegar una expresion que dependa del contexto de runtime.
    if (usesRuntimeVar(expanded)) return false;

    // El evaluador lanza ante funciones/variables desconocidas: en ese caso
    // la expresion sigue siendo texto (fallara o se resolvera mas adelante).
    try {
        float v = evalExpression(expanded, 0.0f, 0.0f, 0.0f, 0.0f);
        if (!std::isfinite(v)) return false;
        out = v;
        return true;
    } catch (...) {
        return false;
    }
}

std::unique_ptr<AstSdf> Resolver::cloneSubstSdf(const AstSdf& src,
    const std::unordered_map<std::string, std::string>& subst) const
{
    auto combine = [&](const std::string& t) -> std::string {
        std::string r = substExprText(t, subst);
        r = expandExpression(r);
        return r;
    };
    auto out = std::make_unique<AstSdf>();
    if (std::holds_alternative<SdfPrimitive>(src.data)) {
        auto& prim = std::get<SdfPrimitive>(src.data);
        SdfPrimitive new_prim;
        new_prim.type = prim.type;
        for (auto& p : prim.params) {
            ExprNode e;
            e.text = combine(p.text);
            new_prim.params.push_back(e);
        }
        out->data = std::move(new_prim);
    } else if (std::holds_alternative<SdfOperator>(src.data)) {
        auto& op = std::get<SdfOperator>(src.data);
        SdfOperator new_op;
        new_op.type = op.type;
        if (op.left) new_op.left = cloneSubstSdf(*op.left, subst);
        if (op.right) new_op.right = cloneSubstSdf(*op.right, subst);
        if (op.k) new_op.k = ExprNode{combine(op.k->text)};
        if (op.param) new_op.param = ExprNode{combine(op.param->text)};
        out->data = std::move(new_op);
    }
    return out;
}

void Resolver::resolveDefs() {
    for (auto& d : ast.defs) {
        if (d.params.empty()) {
            u32 id = resolveNode(*d.node, rih.defs);
            def_map[d.name] = id;
        } else {
            ast_def_map[d.name] = &d;
        }
    }
}

} // namespace herm
