#include "herm_render.h"
#include "herm_eval.h"
#include <cmath>
#include <cctype>
#include <limits>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <stack>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../3rdparty/stb_image_write.h"

namespace herm {

// ============================================================================
// DL-32: validacion del path --render
// ============================================================================

namespace {

bool isIdentChar(char c) {
    return std::isalnum((unsigned char)c) || c == '_';
}

// Variables que solo existen por muestra. `w`/`t` quedan fuera: son tiempo y
// evaluateFrame puede plegarlas. Las de sombreado tambien se rechazan porque
// evalSdfTree no las proporciona.
bool usesSpatialVar(const std::string& text) {
    static const char* vars[] = {
        "x", "y", "z",
        "nx", "ny", "nz",
        "vx", "vy", "vz",
        "lx", "ly", "lz"
    };
    size_t i = 0;
    while (i < text.size()) {
        if (std::isalpha((unsigned char)text[i]) || text[i] == '_') {
            size_t start = i;
            while (i < text.size() && isIdentChar(text[i])) i++;
            std::string name = text.substr(start, i - start);
            for (const char* v : vars) {
                if (name == v) return true;
            }
        } else {
            i++;
        }
    }
    return false;
}

std::string lineSuffix(int line) {
    return line > 0 ? " (linea " + std::to_string(line) + ")" : "";
}

} // namespace

void validateRenderCompatibility(const Rih& rih) {
    auto checkNode = [](const Node& n) {
        for (const auto& sdf : n.sdf_nodes) {
            // CUSTOM se evalua por muestra en evalSdfTree; el resto de
            // primitivas lee params[i].constant.
            if (sdf.type == SdfType::CUSTOM) continue;
            for (int i = 0; i < 4; i++) {
                const Expr& p = sdf.params[i];
                if (!p.is_expr || !usesSpatialVar(p.expression)) continue;
                throw std::runtime_error(
                    "el parametro " + std::to_string(i) + " del nodo '" +
                    n.name + "' depende de x/y/z ('" + p.expression +
                    "') y --render no puede evaluarlo por muestra; usa --ont "
                    "o elimina la dependencia espacial" + lineSuffix(p.line));
            }
        }

        auto checkTransform = [&](const Expr (&values)[3], const char* field) {
            for (int i = 0; i < 3; i++) {
                if (!values[i].is_expr || !usesSpatialVar(values[i].expression))
                    continue;
                throw std::runtime_error(
                    "la transformacion '" + std::string(field) + "[" +
                    std::to_string(i) + "]' del nodo '" + n.name +
                    "' depende de x/y/z ('" + values[i].expression +
                    "') y --render no puede evaluarla por muestra" +
                    lineSuffix(values[i].line));
            }
        };
        checkTransform(n.transform.translate, "translate");
        checkTransform(n.transform.rotate, "rotate");
        checkTransform(n.transform.scale, "scale");
    };
    for (const auto& n : rih.nodes) checkNode(n);
    for (const auto& n : rih.defs)  checkNode(n);
}

// ============================================================================
// SDF primitives
// ============================================================================

static float sdSphere(Vec3 p, float r) {
    return sqrtf(p.x * p.x + p.y * p.y + p.z * p.z) - r;
}

static float sdBox(Vec3 p, Vec3 b) {
    Vec3 q = {fabsf(p.x) - b.x, fabsf(p.y) - b.y, fabsf(p.z) - b.z};
    float outside = sqrtf(fmaxf(0, q.x) * fmaxf(0, q.x) +
                          fmaxf(0, q.y) * fmaxf(0, q.y) +
                          fmaxf(0, q.z) * fmaxf(0, q.z));
    float inside = fminf(fmaxf(q.x, fmaxf(q.y, q.z)), 0.0f);
    return outside + inside;
}

static float sdCylinder(Vec3 p, float r, float h) {
    float d = sqrtf(p.x * p.x + p.z * p.z) - r;
    float dy = fabsf(p.y) - h * 0.5f;
    return fminf(fmaxf(d, dy), 0.0f) + sqrtf(fmaxf(0, d) * fmaxf(0, d) + fmaxf(0, dy) * fmaxf(0, dy));
}

static float sdCapsule(Vec3 p, Vec3 a, Vec3 b, float r) {
    Vec3 ab = {b.x - a.x, b.y - a.y, b.z - a.z};
    Vec3 ap = {p.x - a.x, p.y - a.y, p.z - a.z};
    float t = (ap.x * ab.x + ap.y * ab.y + ap.z * ab.z) /
              (ab.x * ab.x + ab.y * ab.y + ab.z * ab.z);
    t = fmaxf(0, fminf(1, t));
    Vec3 closest = {a.x + t * ab.x, a.y + t * ab.y, a.z + t * ab.z};
    Vec3 diff = {p.x - closest.x, p.y - closest.y, p.z - closest.z};
    return sqrtf(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z) - r;
}

static float sdTorus(Vec3 p, float r_major, float r_minor) {
    Vec2 q = {sqrtf(p.x * p.x + p.z * p.z) - r_major, p.y};
    return sqrtf(q.x * q.x + q.y * q.y) - r_minor;
}

static float sdPlane(Vec3 p, Vec3 n, float d) {
    return p.x * n.x + p.y * n.y + p.z * n.z + d;
}

static float sdCone(Vec3 p, float r, float h) {
    Vec2 q = {sqrtf(p.x * p.x + p.z * p.z), p.y};
    float slope = r / h;
    Vec2 tip = {0.0f, h};
    Vec2 line_dir = {tip.x - q.x, tip.y - q.y};
    Vec2 base_center = {0.0f, 0.0f};
    float d_base = sqrtf(q.x * q.x + q.y * q.y) - r;
    float d_tip = sqrtf(q.x * q.x + (q.y - h) * (q.y - h));
    float d_side = (q.y * slope + q.x) / sqrtf(slope * slope + 1);
    if (q.y < 0) return d_base;
    if (q.y > h) return d_tip;
    return d_side;
}

static float sdRoundedBox(Vec3 p, Vec3 size, float r) {
    Vec3 q = {fabsf(p.x) - size.x + r, fabsf(p.y) - size.y + r, fabsf(p.z) - size.z + r};
    float outside = sqrtf(fmaxf(0, q.x) * fmaxf(0, q.x) +
                          fmaxf(0, q.y) * fmaxf(0, q.y) +
                          fmaxf(0, q.z) * fmaxf(0, q.z));
    float inside = fminf(fmaxf(q.x, fmaxf(q.y, q.z)), 0.0f);
    return outside + inside - r;
}

static float sdOctahedron(Vec3 p, float r) {
    p.x = fabsf(p.x); p.y = fabsf(p.y); p.z = fabsf(p.z);
    float d = p.x + p.y + p.z - r;
    if (d < 0) {
        Vec3 q = {fmaxf(0, p.x - r), fmaxf(0, p.y - r), fmaxf(0, p.z - r)};
        return sqrtf(q.x * q.x + q.y * q.y + q.z * q.z);
    }
    return d * 0.57735027f;
}

// ============================================================================
// SDF operators
// ============================================================================

static float opUnion(float a, float b) { return fminf(a, b); }
static float opSubtract(float a, float b) { return fmaxf(a, -b); }
static float opIntersect(float a, float b) { return fmaxf(a, b); }

static float opSmoothUnion(float a, float b, float k) {
    float h = fmaxf(k - fabsf(a - b), 0.0f) / k;
    return fminf(a, b) - h * h * k * (1.0f / 6.0f);
}

static float opSmoothSubtract(float a, float b, float k) {
    float h = fmaxf(k - fabsf(-a - b), 0.0f) / k;
    return fmaxf(-a, b) + h * h * k * (1.0f / 6.0f);
}

static float opSmoothIntersect(float a, float b, float k) {
    float h = fmaxf(k - fabsf(a - b), 0.0f) / k;
    return fmaxf(a, b) + h * h * k * (1.0f / 6.0f);
}

static Vec3 opRepeatDomain(Vec3 p, float spacing) {
    if (spacing <= 0) return p;
    return {p.x - roundf(p.x / spacing) * spacing,
            p.y - roundf(p.y / spacing) * spacing,
            p.z - roundf(p.z / spacing) * spacing};
}

static Vec3 opTwist(Vec3 p, float angle_per_unit) {
    float angle = p.y * angle_per_unit * 3.14159265f / 180.0f;
    float c = cosf(angle), s = sinf(angle);
    float x = p.x * c - p.z * s;
    float z = p.x * s + p.z * c;
    p.x = x; p.z = z;
    return p;
}

static Vec3 opElongate(Vec3 p, Vec3 h) {
    p.x -= fmaxf(-h.x, fminf(p.x, h.x));
    p.y -= fmaxf(-h.y, fminf(p.y, h.y));
    p.z -= fmaxf(-h.z, fminf(p.z, h.z));
    return p;
}

static float opDisplace(float d, Vec3 p, float amplitude) {
    float noise = sinf(p.x * 5.0f + p.y * 3.0f + p.z * 7.0f) * 0.5f
                + sinf(p.x * 11.0f + p.y * 13.0f) * 0.3f;
    return d + noise * amplitude;
}

// ============================================================================
// SDF tree evaluator
// ============================================================================

float evalSdfTree(const std::vector<SdfNode>& nodes, u32 idx, Vec3 p, float w) {
    if (idx >= (u32)nodes.size()) return 1e9;
    const auto& node = nodes[idx];

    switch (node.type) {
        case SdfType::SPHERE:      return sdSphere(p, node.params[0].constant);
        case SdfType::BOX:         return sdBox(p, {node.params[0].constant, node.params[1].constant, node.params[2].constant});
        case SdfType::CYLINDER:    return sdCylinder(p, node.params[0].constant, node.params[1].constant);
        case SdfType::CAPSULE:     return sdCapsule(p, {0,0,0}, {0,node.params[0].constant,0}, node.params[1].constant);
        case SdfType::TORUS:       return sdTorus(p, node.params[0].constant, node.params[1].constant);
        case SdfType::PLANE:       return sdPlane(p, {node.params[0].constant,node.params[1].constant,node.params[2].constant}, node.params[3].constant);
        case SdfType::CONE:        return sdCone(p, node.params[0].constant, node.params[1].constant);
        case SdfType::ROUNDED_BOX: return sdRoundedBox(p, {node.params[0].constant,node.params[1].constant,node.params[2].constant}, node.params[3].constant);
        case SdfType::OCTAHEDRON:  return sdOctahedron(p, node.params[0].constant);

        case SdfType::UNION: {
            float a = evalSdfTree(nodes, node.child_a.value_or(0xFFFFFFFF), p, w);
            float b = evalSdfTree(nodes, node.child_b.value_or(0xFFFFFFFF), p, w);
            return opUnion(a, b);
        }
        case SdfType::SUBTRACT: {
            float a = evalSdfTree(nodes, node.child_a.value_or(0xFFFFFFFF), p, w);
            float b = evalSdfTree(nodes, node.child_b.value_or(0xFFFFFFFF), p, w);
            return opSubtract(a, b);
        }
        case SdfType::INTERSECT: {
            float a = evalSdfTree(nodes, node.child_a.value_or(0xFFFFFFFF), p, w);
            float b = evalSdfTree(nodes, node.child_b.value_or(0xFFFFFFFF), p, w);
            return opIntersect(a, b);
        }
        case SdfType::SMOOTH_UNION: {
            float a = evalSdfTree(nodes, node.child_a.value_or(0xFFFFFFFF), p, w);
            float b = evalSdfTree(nodes, node.child_b.value_or(0xFFFFFFFF), p, w);
            return opSmoothUnion(a, b, node.k.value_or(0.1f));
        }
        case SdfType::SMOOTH_SUBTRACT: {
            float a = evalSdfTree(nodes, node.child_a.value_or(0xFFFFFFFF), p, w);
            float b = evalSdfTree(nodes, node.child_b.value_or(0xFFFFFFFF), p, w);
            return opSmoothSubtract(a, b, node.k.value_or(0.1f));
        }
        case SdfType::SMOOTH_INTERSECT: {
            float a = evalSdfTree(nodes, node.child_a.value_or(0xFFFFFFFF), p, w);
            float b = evalSdfTree(nodes, node.child_b.value_or(0xFFFFFFFF), p, w);
            return opSmoothIntersect(a, b, node.k.value_or(0.1f));
        }
        case SdfType::SPHERE4D: {
            float r = node.params[0].constant;
            float w_off = node.params[1].constant;
            float r_eff = sqrtf(fmaxf(r * r - w_off * w_off, 0));
            return sdSphere(p, r_eff);
        }
        case SdfType::TESSERACT: {
            return sdRoundedBox(p, {node.params[0].constant, node.params[1].constant, node.params[2].constant}, node.params[3].constant);
        }
        case SdfType::CUSTOM: {
            if (!node.params[0].is_expr || node.params[0].expression.empty()) return 1e9f;
            return evalExpression(node.params[0].expression, p.x, p.y, p.z, w);
        }
        case SdfType::REPEAT: {
            Vec3 q = opRepeatDomain(p, node.spacing.value_or(1.0f));
            return evalSdfTree(nodes, node.child_a.value_or(0xFFFFFFFF), q, w);
        }
        case SdfType::TWIST: {
            Vec3 q = opTwist(p, node.params[0].constant);
            return evalSdfTree(nodes, node.child_a.value_or(0xFFFFFFFF), q, w);
        }
        case SdfType::ELONGATE: {
            Vec3 q = opElongate(p, {node.params[0].constant, node.params[1].constant, node.params[2].constant});
            return evalSdfTree(nodes, node.child_a.value_or(0xFFFFFFFF), q, w);
        }
        case SdfType::DISPLACE: {
            float d = evalSdfTree(nodes, node.child_a.value_or(0xFFFFFFFF), p, w);
            if (node.displace_func.has_value() && !node.displace_func->empty()) {
                return d + evalExpression(*node.displace_func, p.x, p.y, p.z, w);
            }
            return opDisplace(d, p, node.params[0].constant);
        }
        default:
            return 1e9;
    }
}

// ============================================================================
// Transform helpers
// ============================================================================

static Vec3 applyTransform(Vec3 p, const Transform& t) {
    // translate
    p.x -= t.translate[0].constant;
    p.y -= t.translate[1].constant;
    p.z -= t.translate[2].constant;
    // rotate (XYZ Euler)
    float rx = t.rotate[0].constant * 3.14159265f / 180.0f;
    float ry = t.rotate[1].constant * 3.14159265f / 180.0f;
    float rz = t.rotate[2].constant * 3.14159265f / 180.0f;
    float cx = cosf(rx), sx = sinf(rx);
    float cy = cosf(ry), sy = sinf(ry);
    float cz = cosf(rz), sz = sinf(rz);
    // Rotate X
    float y1 = p.y * cx - p.z * sx;
    float z1 = p.y * sx + p.z * cx;
    p.y = y1; p.z = z1;
    // Rotate Y
    float x1 = p.x * cy + p.z * sy;
    float z2 = -p.x * sy + p.z * cy;
    p.x = x1; p.z = z2;
    // Rotate Z
    float x2 = p.x * cz - p.y * sz;
    float y2 = p.x * sz + p.y * cz;
    p.x = x2; p.y = y2;
    // scale
    float sx_f = t.scale[0].constant;
    float sy_f = t.scale[1].constant;
    float sz_f = t.scale[2].constant;
    if (sx_f != 0) p.x /= sx_f;
    if (sy_f != 0) p.y /= sy_f;
    if (sz_f != 0) p.z /= sz_f;
    return p;
}

// ============================================================================
// Scene evaluator — evaluate all nodes at world point p
// ============================================================================

struct EvalResult {
    float dist = 1e9f;
    u32 material_id = 0;
    float vol_density = 0.0f;    // densidad acumulada en volumen (0 = fuera)
    u32 vol_material_id = 0;
    float vol_emission_r = 0.0f;
    float vol_emission_g = 0.0f;
    float vol_emission_b = 0.0f;
};

struct FlatNode {
    u32 node_idx;
    Transform xform;
    u32 material_id;
    bool is_sdf;
    bool is_instance;
    bool is_volume;
    u32 def_id;
};

static void composeTransform(Transform& acc, const Transform& t) {
    acc.translate[0].constant += t.translate[0].constant;
    acc.translate[1].constant += t.translate[1].constant;
    acc.translate[2].constant += t.translate[2].constant;
    acc.rotate[0].constant += t.rotate[0].constant;
    acc.rotate[1].constant += t.rotate[1].constant;
    acc.rotate[2].constant += t.rotate[2].constant;
    acc.scale[0].constant *= t.scale[0].constant;
    acc.scale[1].constant *= t.scale[1].constant;
    acc.scale[2].constant *= t.scale[2].constant;
}

static std::vector<FlatNode> flattenNodes(const Rih& rih) {
    std::vector<FlatNode> result;

    // Build id→node index
    std::unordered_map<u32, const Node*> node_map;
    for (auto& n : rih.nodes) node_map[n.id] = &n;
    for (auto& d : rih.defs)  node_map[d.id] = &d;

    // Build child→parent map for group hierarchy
    std::unordered_map<u32, u32> child_parent;
    for (auto& n : rih.nodes)
        if (n.type == NodeType::GROUP)
            for (auto cid : n.children)
                child_parent[cid] = n.id;

    // Accumulate transform up the parent chain (leaf → root)
    auto accumulate = [&](u32 id) -> Transform {
        std::vector<const Node*> chain;
        u32 cur = id;
        while (true) {
            auto it = node_map.find(cur);
            if (it == node_map.end()) break;
            chain.push_back(it->second);
            auto pit = child_parent.find(cur);
            if (pit == child_parent.end()) break;
            cur = pit->second;
        }
        // chain = [leaf, parent, grandparent, ..., root]
        // Apply from root down to leaf
        Transform acc = {};
        acc.scale[0].constant = 1; acc.scale[1].constant = 1; acc.scale[2].constant = 1;
        for (int i = (int)chain.size() - 1; i >= 0; i--)
            composeTransform(acc, chain[i]->transform);
        return acc;
    };

    // Flatten all non-group nodes with accumulated transforms
    for (auto& n : rih.nodes) {
        if (n.type == NodeType::GROUP) continue;
        FlatNode fn;
        fn.node_idx = n.id;
        fn.xform = accumulate(n.id);
        fn.material_id = n.material_id.value_or(0);
        fn.is_sdf = (n.type == NodeType::SDF);
        fn.is_instance = (n.type == NodeType::INSTANCE);
        fn.is_volume = (n.mode == NodeMode::VOLUME);
        fn.def_id = n.def_id.value_or(UINT32_MAX);
        result.push_back(fn);
    }

    // Expand instances with full accumulated def transform + instance override
    for (auto& n : rih.nodes) {
        if (n.type != NodeType::INSTANCE || !n.def_id.has_value()) continue;
        u32 did = *n.def_id;
        if (did < (u32)rih.defs.size()) {
            auto& def = rih.defs[did];
            FlatNode dfn;
            dfn.node_idx = def.id;
            dfn.xform = accumulate(def.id);
            dfn.material_id = def.material_id.value_or(n.material_id.value_or(0));
            dfn.is_sdf = (def.type == NodeType::SDF);
            dfn.is_instance = false;
            dfn.is_volume = (def.mode == NodeMode::VOLUME);
            dfn.def_id = UINT32_MAX;
            // Compose instance transform on top of def's accumulated transform
            composeTransform(dfn.xform, n.transform);
            result.push_back(dfn);
        }
    }

    return result;
}

static EvalResult evalScene(const Rih& rih, Vec3 p, const std::vector<FlatNode>& flat_nodes, float w) {
    EvalResult best;
    for (auto& fn : flat_nodes) {
        if (!fn.is_sdf) continue;
        Vec3 local_p = applyTransform(p, fn.xform);
        const Node* node = nullptr;
        for (auto& n : rih.nodes) {
            if (n.id == fn.node_idx) { node = &n; break; }
        }
        if (!node) {
            for (auto& d : rih.defs) {
                if (d.id == fn.node_idx) { node = &d; break; }
            }
        }
        if (!node || node->sdf_nodes.empty()) continue;
        float d = evalSdfTree(node->sdf_nodes, 0, local_p, w);
        float min_scale = fminf(fminf(fn.xform.scale[0].constant,
                                      fn.xform.scale[1].constant),
                                      fn.xform.scale[2].constant);
        d *= min_scale;

        if (fn.is_volume) {
            if (d < 0.5f) {
                float density = fmaxf(0, 1.0f - d * 2.0f);
                density *= node->density_scale;
                if (density > best.vol_density) {
                    best.vol_density = density;
                    best.vol_material_id = fn.material_id;
                    if (fn.material_id < (u32)rih.materials.size()) {
                        auto& mat = rih.materials[fn.material_id];
                        if (mat.tensor[4].constant > 1.0f || mat.tensor[5].constant > 1.0f || mat.tensor[6].constant > 1.0f) {
                            best.vol_emission_r = mat.tensor[4].constant;
                            best.vol_emission_g = mat.tensor[5].constant;
                            best.vol_emission_b = mat.tensor[6].constant;
                        } else {
                            best.vol_emission_r = mat.tensor[4].constant * 0.5f;
                            best.vol_emission_g = mat.tensor[5].constant * 0.5f;
                            best.vol_emission_b = mat.tensor[6].constant * 0.5f;
                        }
                    }
                }
            }
        } else {
            if (d < best.dist) {
                best.dist = d;
                best.material_id = fn.material_id;
            }
        }
    }
    return best;
}

// ============================================================================
// Normal computation (finite differences)
// ============================================================================

static Vec3 calcNormal(const Rih& rih, Vec3 p, const std::vector<FlatNode>& flat_nodes, float w, float eps = 0.001f) {
    auto sdf_at = [&](Vec3 pt) { return evalScene(rih, pt, flat_nodes, w).dist; };
    float d = sdf_at(p);
    Vec3 n;
    n.x = sdf_at({p.x + eps, p.y, p.z}) - d;
    n.y = sdf_at({p.x, p.y + eps, p.z}) - d;
    n.z = sdf_at({p.x, p.y, p.z + eps}) - d;
    float len = sqrtf(n.x * n.x + n.y * n.y + n.z * n.z);
    if (len > 0) { n.x /= len; n.y /= len; n.z /= len; }
    return n;
}

// ============================================================================
// Ray marching
// ============================================================================

struct VolumeResult {
    bool hit;
    ColorRGB accumulated;
    float transmittance;
    float t;
};

static VolumeResult rayMarchVol(const Rih& rih, Vec3 ro, Vec3 rd,
                                const std::vector<FlatNode>& flat_nodes, float w,
                                float t_min, float t_max, int max_steps) {
    float t = t_min;
    ColorRGB vol_color = {0, 0, 0};
    float T = 1.0f;
    bool has_volume = false;

    for (int i = 0; i < max_steps; i++) {
        Vec3 p = {ro.x + rd.x * t, ro.y + rd.y * t, ro.z + rd.z * t};
        auto res = evalScene(rih, p, flat_nodes, w);

        if (res.vol_density > 0.001f) {
            has_volume = true;
            float step = fmaxf(res.dist, 0.05f);
            float density = res.vol_density * 0.5f;
            float absorption = expf(-density * step);
            float scatter = (1.0f - absorption) * T;
            vol_color.r += res.vol_emission_r * scatter;
            vol_color.g += res.vol_emission_g * scatter;
            vol_color.b += res.vol_emission_b * scatter;
            T *= absorption;
            if (T < 0.01f) break;
        }

        if (res.dist < 0.001f) {
            VolumeResult vr;
            vr.hit = true;
            vr.accumulated = vol_color;
            vr.transmittance = T;
            vr.t = t;
            return vr;
        }

        t += fmaxf(res.dist, 0.01f);
        if (t > t_max) break;
    }

    if (has_volume) {
        VolumeResult vr;
        vr.hit = true;
        vr.accumulated = vol_color;
        vr.transmittance = T;
        vr.t = t;
        return vr;
    }
    return {};
}

static HitInfo rayMarch(const Rih& rih, Vec3 ro, Vec3 rd,
                        const std::vector<FlatNode>& flat_nodes, float w,
                        float t_min, float t_max, int max_steps) {
    float t = t_min;
    for (int i = 0; i < max_steps; i++) {
        Vec3 p = {ro.x + rd.x * t, ro.y + rd.y * t, ro.z + rd.z * t};
        auto res = evalScene(rih, p, flat_nodes, w);
        if (res.dist < 0.001f) {
            HitInfo hi;
            hi.hit = true;
            hi.dist = t;
            hi.point = p;
            hi.normal = calcNormal(rih, p, flat_nodes, w);
            hi.material_id = res.material_id;
            return hi;
        }
        t += res.dist;
        if (t > t_max) break;
    }
    return {};
}

// ============================================================================
// Shadow ray
// ============================================================================

static bool shadowRay(const Rih& rih, Vec3 origin, Vec3 light_dir,
                      const std::vector<FlatNode>& flat_nodes, float w,
                      float max_dist, int max_steps) {
    float t = 0.01f;
    for (int i = 0; i < max_steps; i++) {
        Vec3 p = {origin.x + light_dir.x * t, origin.y + light_dir.y * t, origin.z + light_dir.z * t};
        auto res = evalScene(rih, p, flat_nodes, w);
        if (res.dist < 0.001f) return true;
        t += res.dist;
        if (t > max_dist) break;
    }
    return false;
}

// ============================================================================
// Shading
// ============================================================================

static Rgba shade(const Material& mat, Vec3 normal, Vec3 view_dir, Vec3 light_dir,
                   ColorRGB bg, float shadow) {
    Vec3 l = light_dir;
    float ln = fmaxf(0, normal.x * l.x + normal.y * l.y + normal.z * l.z);
    Vec3 half_vec = {(l.x + view_dir.x) * 0.5f, (l.y + view_dir.y) * 0.5f, (l.z + view_dir.z) * 0.5f};
    float hn = fmaxf(0, normal.x * half_vec.x + normal.y * half_vec.y + normal.z * half_vec.z);

    float ambient = 0.1f;
    float diffuse = 0.6f * ln * shadow;
    float spec = 0.3f * powf(hn, 16.0f) * shadow;

    float r = mat.tensor[4].constant * (ambient + diffuse) + spec;
    float g = mat.tensor[5].constant * (ambient + diffuse) + spec;
    float b = mat.tensor[6].constant * (ambient + diffuse) + spec;

    float ma = mat.tensor[7].constant;
    if (ma < 1.0f) {
        r = r * ma + bg.r * (1 - ma);
        g = g * ma + bg.g * (1 - ma);
        b = b * ma + bg.b * (1 - ma);
    }

    auto clamp = [](float v) -> uint8_t {
        return (uint8_t)fminf(255, fmaxf(0, v * 255));
    };
    return {clamp(r), clamp(g), clamp(b), 255};
}

// ============================================================================
// Render
// ============================================================================

std::vector<Rgba> renderRih(const Rih& rih, int width, int height) {
    std::vector<Rgba> pixels((size_t)width * height);

    auto flat_nodes = flattenNodes(rih);

    Camera cam = rih.scene.camera;
    Vec3 ro = cam.position;
    Vec3 target = cam.target;
    Vec3 up_v = cam.up;

    Vec3 forward = {target.x - ro.x, target.y - ro.y, target.z - ro.z};
    float fwd_len = sqrtf(forward.x * forward.x + forward.y * forward.y + forward.z * forward.z);
    if (fwd_len > 0) { forward.x /= fwd_len; forward.y /= fwd_len; forward.z /= fwd_len; }

    Vec3 right;
    float rl = forward.x * up_v.x + forward.y * up_v.y + forward.z * up_v.z;
    right.x = up_v.x - forward.x * rl;
    right.y = up_v.y - forward.y * rl;
    right.z = up_v.z - forward.z * rl;
    float rt_len = sqrtf(right.x * right.x + right.y * right.y + right.z * right.z);
    if (rt_len > 0) { right.x /= rt_len; right.y /= rt_len; right.z /= rt_len; }

    Vec3 up;
    up.x = right.y * forward.z - right.z * forward.y;
    up.y = right.z * forward.x - right.x * forward.z;
    up.z = right.x * forward.y - right.y * forward.x;

    float fov_scale = tanf(cam.fov * 0.5f * 3.14159265f / 180.0f);
    float aspect = (float)width / (float)height;

    ColorRGB bg = rih.scene.background;
    float w = rih.scene.w_min; // Current frame time (W)

    // Build light list: parse lights or use default directional
    auto getLights = [&]() -> std::vector<Light> {
        if (!rih.lights.empty()) return rih.lights;
        // Default directional light for backward compatibility
        Light def;
        def.id = 0;
        def.name = "default";
        def.type = LightType::DIRECTIONAL;
        def.direction = {0.3f, 0.8f, 0.5f};
        float ld_len = sqrtf(def.direction.x * def.direction.x + def.direction.y * def.direction.y + def.direction.z * def.direction.z);
        if (ld_len > 0) { def.direction.x /= ld_len; def.direction.y /= ld_len; def.direction.z /= ld_len; }
        def.color = {1,1,1};
        def.intensity = 1.0f;
        return {def};
    };
    auto lights = getLights();

    // Compute shadow + shading per light
    auto shadeWithLights = [&](Vec3 point, Vec3 normal, Vec3 view_dir, const Material& mat) -> Rgba {
        auto getTensorVal = [&](int idx) -> float {
            if (mat.tensor[idx].is_expr) return evalExpression(mat.tensor[idx].expression, point.x, point.y, point.z, w);
            return mat.tensor[idx].constant;
        };

        float mr = getTensorVal(4);
        float mg = getTensorVal(5);
        float mb = getTensorVal(6);
        float ma = getTensorVal(7); // if needed for opacity

        float total_r = 0, total_g = 0, total_b = 0;
        for (auto& L : lights) {
            Vec3 l;
            float atten = 1.0f;
            float dist = 1e9f;

            if (L.type == LightType::POINT) {
                l.x = L.position.x - point.x;
                l.y = L.position.y - point.y;
                l.z = L.position.z - point.z;
                dist = sqrtf(l.x * l.x + l.y * l.y + l.z * l.z);
                if (dist > 0.001f) { l.x /= dist; l.y /= dist; l.z /= dist; } else continue;
                atten = 1.0f / (1.0f + L.falloff * dist * dist);
            } else {
                l = L.direction;
            }

            // Shadow only for directional lights
            float shadow = 1.0f;
            if (L.type == LightType::DIRECTIONAL) {
                Vec3 so = {point.x + normal.x * 0.01f, point.y + normal.y * 0.01f, point.z + normal.z * 0.01f};
                if (shadowRay(rih, so, l, flat_nodes, w, 5.0f, 32))
                    shadow = 0.3f;
            }

            float ln = fmaxf(0, normal.x * l.x + normal.y * l.y + normal.z * l.z);
            Vec3 half_vec = {(l.x + view_dir.x) * 0.5f, (l.y + view_dir.y) * 0.5f, (l.z + view_dir.z) * 0.5f};
            float hn = fmaxf(0, normal.x * half_vec.x + normal.y * half_vec.y + normal.z * half_vec.z);

            float ambient = 0.1f;
            float diffuse = 0.6f * ln * shadow;
            float spec = 0.3f * powf(hn, 16.0f) * shadow;

            float lr = mr * (ambient + diffuse) + spec;
            float lg = mg * (ambient + diffuse) + spec;
            float lb = mb * (ambient + diffuse) + spec;

            lr *= L.color.r * L.intensity * atten;
            lg *= L.color.g * L.intensity * atten;
            lb *= L.color.b * L.intensity * atten;

            total_r += lr;
            total_g += lg;
            total_b += lb;
        }

        // Transmission / Opacity (using tensor alpha channel, index 7)
        if (ma < 1.0f) {
            total_r = total_r * ma + bg.r * (1 - ma);
            total_g = total_g * ma + bg.g * (1 - ma);
            total_b = total_b * ma + bg.b * (1 - ma);
        }

        auto clamp = [](float v) -> uint8_t {
            return (uint8_t)fminf(255, fmaxf(0, v * 255));
        };
        return {clamp(total_r), clamp(total_g), clamp(total_b), 255};
    };

    // Check if scene has any volume nodes
    bool has_volumes = false;
    for (auto& fn : flat_nodes) {
        if (fn.is_volume) { has_volumes = true; break; }
    }

    for (int py = 0; py < height; py++) {
        for (int px = 0; px < width; px++) {
            float u = (2.0f * (px + 0.5f) / (float)width - 1.0f) * aspect * fov_scale;
            float v = (1.0f - 2.0f * (py + 0.5f) / (float)height) * fov_scale;

            Vec3 rd = {
                forward.x + right.x * u + up.x * v,
                forward.y + right.y * u + up.y * v,
                forward.z + right.z * u + up.z * v
            };
            float rd_len = sqrtf(rd.x * rd.x + rd.y * rd.y + rd.z * rd.z);
            if (rd_len > 0) { rd.x /= rd_len; rd.y /= rd_len; rd.z /= rd_len; }

            auto clamp = [](float v) -> uint8_t {
                return (uint8_t)fminf(255, fmaxf(0, v * 255));
            };

            // Volume path
            if (has_volumes) {
                auto vol = rayMarchVol(rih, ro, rd, flat_nodes, w, 0.01f, 50.0f, 128);
                if (vol.hit) {
                    // Solid surface hit behind/in volume
                    if (vol.t > 0.01f && vol.transmittance > 0.01f) {
                        Vec3 sp = {ro.x + rd.x * vol.t, ro.y + rd.y * vol.t, ro.z + rd.z * vol.t};
                        auto sres = evalScene(rih, sp, flat_nodes, w);
                        if (sres.dist < 0.001f) {
                            Vec3 sn = calcNormal(rih, sp, flat_nodes, w);
                            Vec3 view_dir = {-rd.x, -rd.y, -rd.z};
                            const Material* mat = nullptr;
                            if (sres.material_id < (u32)rih.materials.size())
                                mat = &rih.materials[sres.material_id];
                            if (!mat) { static Material dm; dm.tensor[4].constant = 1.0f; dm.tensor[5].constant = 1.0f; dm.tensor[6].constant = 1.0f; dm.tensor[7].constant = 1.0f; mat = &dm; }
                            auto sc = shadeWithLights(sp, sn, view_dir, *mat);
                            // Blend: vol_contrib + T * solid_contrib
                            float fr = vol.accumulated.r + vol.transmittance * sc.r / 255.0f;
                            float fg = vol.accumulated.g + vol.transmittance * sc.g / 255.0f;
                            float fb = vol.accumulated.b + vol.transmittance * sc.b / 255.0f;
                            pixels[py * width + px] = {clamp(fr), clamp(fg), clamp(fb), 255};
                            continue;
                        }
                    }
                    // Only volume (no solid hit or fully absorbed)
                    float T = vol.transmittance;
                    float vr = vol.accumulated.r + T * bg.r;
                    float vg = vol.accumulated.g + T * bg.g;
                    float vb = vol.accumulated.b + T * bg.b;
                    pixels[py * width + px] = {clamp(vr), clamp(vg), clamp(vb), 255};
                    continue;
                }
            }

            // Solid path (fallback)
            auto hit = rayMarch(rih, ro, rd, flat_nodes, w, 0.01f, 50.0f, 128);

            if (hit.hit) {
                Vec3 view_dir = {-rd.x, -rd.y, -rd.z};
                const Material* mat = nullptr;
                if (hit.material_id < (u32)rih.materials.size())
                    mat = &rih.materials[hit.material_id];
                if (!mat) {
                    static Material default_mat;
                    default_mat.tensor[4].constant = 1.0f;
                    default_mat.tensor[5].constant = 1.0f;
                    default_mat.tensor[6].constant = 1.0f;
                    default_mat.tensor[7].constant = 1.0f;
                    mat = &default_mat;
                }
                pixels[py * width + px] = shadeWithLights(hit.point, hit.normal, view_dir, *mat);
            } else {
                pixels[py * width + px] = {clamp(bg.r), clamp(bg.g), clamp(bg.b), 255};
            }
        }
    }

    return pixels;
}

// ============================================================================
// GTE writer — writes 8x8 blocks as GTE slots
// ============================================================================

bool writeGte(const Rih& rih, const std::vector<Rgba>& pixels,
              int width, int height, const std::string& path) {
    std::ofstream out(path);
    if (!out) return false;

    out << "// GTE export from herm compiler\n";
    out << "// Scene: " << rih.scene.t_unit << "\n";
    out << "\n";
    out << "new eje Y = " << height << "\n";
    out << "new eje X = " << width << "\n";
    out << "\n";
    out << "present() {\n";

    int block_size = 8;
    int blocks_x = (width + block_size - 1) / block_size;
    int blocks_y = (height + block_size - 1) / block_size;

    int slot_idx = 0;
    for (int by = 0; by < blocks_y; by++) {
        for (int bx = 0; bx < blocks_x; bx++) {
            // Compute average color of this block
            unsigned sum_r = 0, sum_g = 0, sum_b = 0, sum_a = 0, count = 0;
            int x0 = bx * block_size;
            int y0 = by * block_size;
            int x1 = std::min(x0 + block_size, width);
            int y1 = std::min(y0 + block_size, height);

            for (int py = y0; py < y1; py++) {
                for (int px = x0; px < x1; px++) {
                    auto& p = pixels[py * width + px];
                    sum_r += p.r;
                    sum_g += p.g;
                    sum_b += p.b;
                    sum_a += p.a;
                    count++;
                }
            }

            if (count == 0) continue;
            int ar = (int)(sum_r / count);
            int ag = (int)(sum_g / count);
            int ab = (int)(sum_b / count);
            int aa = (int)(sum_a / count);

            if (aa == 0) continue; // Skip fully transparent blocks

            out << "  slot b" << slot_idx << " {\n";
            out << "    en: y = " << y0 << ".." << (y1 - 1)
                << ", x = " << x0 << ".." << (x1 - 1) << ";\n";
            out << "    relleno { R: " << ar << "; G: " << ag
                << "; B: " << ab << "; A: " << aa << "; }\n";
            out << "  }\n";
            slot_idx++;
        }
    }

    out << "}\n";
    return true;
}

// ============================================================================
// PNG writer — direct PNG output via stb_image_write
// ============================================================================

bool writePng(const std::vector<Rgba>& pixels,
              int width, int height, const std::string& path) {
    // Convert Rgba vector to raw RGBA byte array
    std::vector<uint8_t> raw((size_t)width * height * 4);
    for (int i = 0; i < width * height; i++) {
        raw[i * 4 + 0] = pixels[i].r;
        raw[i * 4 + 1] = pixels[i].g;
        raw[i * 4 + 2] = pixels[i].b;
        raw[i * 4 + 3] = pixels[i].a;
    }
    int ok = stbi_write_png(path.c_str(), width, height, 4, raw.data(), width * 4);
    return ok != 0;
}

} // namespace herm
