#include <stdexcept>
#include "herm_resolver.h"

namespace herm {

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
    try {
        size_t pos;
        float val = std::stof(text, &pos);
        if (pos == text.size()) return val;
    } catch (...) {}
    return 0.0f;
}

Vec3 Resolver::evalVec3(const Vec3Expr& vec) const {
    return {evalExpr(vec.x), evalExpr(vec.y), evalExpr(vec.z)};
}

ColorRGB Resolver::evalColor3(const Color3& col) const {
    return {evalExpr(col.r), evalExpr(col.g), evalExpr(col.b)};
}

Expr Resolver::toExpr(const ExprNode& node, f32 default_val) const {
    Expr e;
    if (node.text.empty()) {
        e.is_expr = false;
        e.constant = default_val;
        return e;
    }
    std::string text = expandExpression(node.text);
    try {
        size_t pos;
        float val = std::stof(text, &pos);
        if (pos == text.size()) {
            e.is_expr = false;
            e.constant = val;
            return e;
        }
    } catch (...) {}
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
            mat.tensor[i] = toExpr(ExprNode{}, 0.0f);
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

    sdf_list.push_back(node);
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
    for (auto& [param, arg] : subst) {
        size_t pos = 0;
        while ((pos = t.find(param, pos)) != std::string::npos) {
            bool bl = (pos == 0 || !std::isalnum((unsigned char)t[pos - 1]));
            bool br = (pos + param.size() >= t.size() || !std::isalnum((unsigned char)t[pos + param.size()]));
            if (bl && br) {
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
    if (depth > 16) return text; // Límite de recursión para evitar cuelgues

    std::string t = substExprText(text, let_table);
    
    std::string res;
    size_t i = 0;
    bool changed = false;
    while (i < t.size()) {
        if (std::isalpha(t[i]) || t[i] == '_') {
            size_t start = i;
            while (i < t.size() && (std::isalnum(t[i]) || t[i] == '_')) i++;
            std::string name = t.substr(start, i - start);
            
            size_t j = i;
            while (j < t.size() && std::isspace(t[j])) j++;
            
            if (j < t.size() && t[j] == '(' && fn_table.count(name)) {
                const AstFn* fn = fn_table.at(name);
                j++; // skip '('
                
                std::vector<std::string> args;
                std::string current_arg;
                int parens = 0;
                
                while (j < t.size()) {
                    if (t[j] == '(') parens++;
                    else if (t[j] == ')') {
                        if (parens == 0) {
                            args.push_back(current_arg);
                            j++;
                            break;
                        }
                        parens--;
                    }
                    else if (t[j] == ',' && parens == 0) {
                        args.push_back(current_arg);
                        current_arg = "";
                        j++;
                        continue;
                    }
                    current_arg += t[j];
                    j++;
                }
                
                std::unordered_map<std::string, std::string> param_subst;
                for (size_t k = 0; k < std::min(fn->params.size(), args.size()); k++) {
                    param_subst[fn->params[k]] = "(" + args[k] + ")";
                }
                
                std::string expanded = substExprText(fn->body_expr, param_subst);
                res += "(" + expanded + ")";
                i = j;
                changed = true;
                continue;
            } else {
                res += name;
            }
        } else {
            res += t[i];
            i++;
        }
    }
    
    if (changed) {
        return expandExpression(res, depth + 1);
    }
    return res;
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
