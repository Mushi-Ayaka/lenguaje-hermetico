#include "renderer.h"
#include <cmath>
#include <limits>
#include <algorithm>
#include <vector>

using namespace herm;

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
    float d_side = (q.y * slope + q.x) / sqrtf(slope * slope + 1);
    float d_base = sqrtf(q.x * q.x + q.y * q.y) - r;
    float d_tip = sqrtf(q.x * q.x + (q.y - h) * (q.y - h));
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

static float evalSdfTree(const std::vector<SdfNode>& nodes, u32 idx, Vec3 p) {
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

        case SdfType::SPHERE4D: {
            float r = node.params[0].constant;
            float w_off = node.params[1].constant;
            float r_eff = sqrtf(fmaxf(r * r - w_off * w_off, 0));
            return sdSphere(p, r_eff);
        }
        case SdfType::TESSERACT: {
            return sdRoundedBox(p, {node.params[0].constant, node.params[1].constant, node.params[2].constant}, node.params[3].constant);
        }

        case SdfType::UNION: {
            float a = evalSdfTree(nodes, node.child_a.value_or(0xFFFFFFFF), p);
            float b = evalSdfTree(nodes, node.child_b.value_or(0xFFFFFFFF), p);
            return opUnion(a, b);
        }
        case SdfType::SUBTRACT: {
            float a = evalSdfTree(nodes, node.child_a.value_or(0xFFFFFFFF), p);
            float b = evalSdfTree(nodes, node.child_b.value_or(0xFFFFFFFF), p);
            return opSubtract(a, b);
        }
        case SdfType::INTERSECT: {
            float a = evalSdfTree(nodes, node.child_a.value_or(0xFFFFFFFF), p);
            float b = evalSdfTree(nodes, node.child_b.value_or(0xFFFFFFFF), p);
            return opIntersect(a, b);
        }
        case SdfType::SMOOTH_UNION: {
            float a = evalSdfTree(nodes, node.child_a.value_or(0xFFFFFFFF), p);
            float b = evalSdfTree(nodes, node.child_b.value_or(0xFFFFFFFF), p);
            return opSmoothUnion(a, b, node.k.value_or(0.1f));
        }
        case SdfType::SMOOTH_SUBTRACT: {
            float a = evalSdfTree(nodes, node.child_a.value_or(0xFFFFFFFF), p);
            float b = evalSdfTree(nodes, node.child_b.value_or(0xFFFFFFFF), p);
            return opSmoothSubtract(a, b, node.k.value_or(0.1f));
        }
        case SdfType::SMOOTH_INTERSECT: {
            float a = evalSdfTree(nodes, node.child_a.value_or(0xFFFFFFFF), p);
            float b = evalSdfTree(nodes, node.child_b.value_or(0xFFFFFFFF), p);
            return opSmoothIntersect(a, b, node.k.value_or(0.1f));
        }
        case SdfType::REPEAT: {
            Vec3 q = opRepeatDomain(p, node.spacing.value_or(1.0f));
            return evalSdfTree(nodes, node.child_a.value_or(0xFFFFFFFF), q);
        }
        case SdfType::TWIST: {
            Vec3 q = opTwist(p, node.params[0].constant);
            return evalSdfTree(nodes, node.child_a.value_or(0xFFFFFFFF), q);
        }
        case SdfType::ELONGATE: {
            Vec3 q = opElongate(p, {node.params[0].constant, node.params[1].constant, node.params[2].constant});
            return evalSdfTree(nodes, node.child_a.value_or(0xFFFFFFFF), q);
        }
        case SdfType::DISPLACE: {
            float d = evalSdfTree(nodes, node.child_a.value_or(0xFFFFFFFF), p);
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
    p.x -= t.translate[0].constant;
    p.y -= t.translate[1].constant;
    p.z -= t.translate[2].constant;
    float rx = t.rotate[0].constant * 3.14159265f / 180.0f;
    float ry = t.rotate[1].constant * 3.14159265f / 180.0f;
    float rz = t.rotate[2].constant * 3.14159265f / 180.0f;
    float cx = cosf(rx), sx = sinf(rx);
    float cy = cosf(ry), sy = sinf(ry);
    float cz = cosf(rz), sz = sinf(rz);
    float y1 = p.y * cx - p.z * sx;
    float z1 = p.y * sx + p.z * cx;
    p.y = y1; p.z = z1;
    float x1 = p.x * cy + p.z * sy;
    float z2 = -p.x * sy + p.z * cy;
    p.x = x1; p.z = z2;
    float x2 = p.x * cz - p.y * sz;
    float y2 = p.x * sz + p.y * cz;
    p.x = x2; p.y = y2;
    float sx_f = t.scale[0].constant;
    float sy_f = t.scale[1].constant;
    float sz_f = t.scale[2].constant;
    if (sx_f != 0) p.x /= sx_f;
    if (sy_f != 0) p.y /= sy_f;
    if (sz_f != 0) p.z /= sz_f;
    return p;
}

// ============================================================================
// Scene flattening and evaluation
// ============================================================================

struct EvalResult {
    float dist = 1e9f;
    u32 material_id = 0;
    float vol_density = 0.0f;
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
    bool is_volume;
    NodeMode mode;
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

    std::unordered_map<u32, const Node*> node_map;
    for (auto& n : rih.nodes) node_map[n.id] = &n;
    for (auto& d : rih.defs)  node_map[d.id] = &d;

    std::unordered_map<u32, u32> child_parent;
    for (auto& n : rih.nodes)
        if (n.type == NodeType::GROUP)
            for (auto cid : n.children)
                child_parent[cid] = n.id;

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
        Transform acc = {};
        acc.scale[0].constant = 1; acc.scale[1].constant = 1; acc.scale[2].constant = 1;
        for (int i = (int)chain.size() - 1; i >= 0; i--)
            composeTransform(acc, chain[i]->transform);
        return acc;
    };

    for (auto& n : rih.nodes) {
        if (n.type == NodeType::GROUP) continue;
        FlatNode fn;
        fn.node_idx = n.id;
        fn.xform = accumulate(n.id);
        fn.material_id = n.material_id.value_or(0);
        fn.is_sdf = (n.type == NodeType::SDF);
        fn.mode = n.mode;
        fn.is_volume = (n.mode == NodeMode::VOLUME);
        result.push_back(fn);
    }

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
            dfn.mode = def.mode;
            dfn.is_volume = (def.mode == NodeMode::VOLUME);
            composeTransform(dfn.xform, n.transform);
            result.push_back(dfn);
        }
    }

    return result;
}

static EvalResult evalScene(const Rih& rih, Vec3 p, const std::vector<FlatNode>& flat_nodes) {
    EvalResult best;
    for (auto& fn : flat_nodes) {
        if (!fn.is_sdf) continue;
        Vec3 local_p = applyTransform(p, fn.xform);
        const Node* node = nullptr;
        for (auto& n : rih.nodes) if (n.id == fn.node_idx) { node = &n; break; }
        if (!node) for (auto& d : rih.defs) if (d.id == fn.node_idx) { node = &d; break; }
        if (!node || node->sdf_nodes.empty()) continue;
        float d = evalSdfTree(node->sdf_nodes, 0, local_p);
        float min_scale = fminf(fminf(fn.xform.scale[0].constant,
                                      fn.xform.scale[1].constant),
                                      fn.xform.scale[2].constant);
        d *= min_scale;
        if (d < best.dist) { best.dist = d; best.material_id = fn.material_id; }

        if (fn.is_volume) {
            float density = fmaxf(0, -d) * 0.1f;
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
    }
    return best;
}

// ============================================================================
// Normal via finite differences
// ============================================================================

static Vec3 calcNormal(const Rih& rih, Vec3 p, const std::vector<FlatNode>& flat_nodes, float eps = 0.001f) {
    auto sdf_at = [&](Vec3 pt) { return evalScene(rih, pt, flat_nodes).dist; };
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

struct HitInfo {
    bool hit = false;
    float dist = 0;
    Vec3 point = {};
    Vec3 normal = {};
    u32 material_id = 0;
};

static HitInfo rayMarch(const Rih& rih, Vec3 ro, Vec3 rd,
                        const std::vector<FlatNode>& flat_nodes,
                        float t_min, float t_max, int max_steps) {
    float t = t_min;
    for (int i = 0; i < max_steps; i++) {
        Vec3 p = {ro.x + rd.x * t, ro.y + rd.y * t, ro.z + rd.z * t};
        auto res = evalScene(rih, p, flat_nodes);
        if (res.dist < 0.001f) {
            HitInfo hi;
            hi.hit = true;
            hi.dist = t;
            hi.point = p;
            hi.normal = calcNormal(rih, p, flat_nodes);
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

static bool rayMarchVol(const Rih& rih, Vec3 ro, Vec3 rd,
                         const std::vector<FlatNode>& flat_nodes,
                         float t_min, float t_max, int max_steps,
                         float& out_density, float& out_er, float& out_eg, float& out_eb) {
    out_density = 0; out_er = 0; out_eg = 0; out_eb = 0;
    float step = (t_max - t_min) / fmaxf(1, (float)max_steps);
    float t = t_min;
    for (int i = 0; i < max_steps; i++) {
        Vec3 p = {ro.x + rd.x * t, ro.y + rd.y * t, ro.z + rd.z * t};
        auto res = evalScene(rih, p, flat_nodes);
        float near = fmaxf(res.dist - step * 0.5f, 0);
        if (res.dist < 1.0f && res.vol_density > 0) {
            float local_density = res.vol_density * step;
            float transmittance = expf(-local_density);
            float absorbed = 1.0f - transmittance;
            out_er += absorbed * res.vol_emission_r * (1.0f - out_density);
            out_eg += absorbed * res.vol_emission_g * (1.0f - out_density);
            out_eb += absorbed * res.vol_emission_b * (1.0f - out_density);
            out_density += local_density;
            if (out_density > 0.99f) break;
        }
        t += step;
        if (t > t_max) break;
    }
    return out_density > 0.01f;
}

static bool shadowRay(const Rih& rih, Vec3 origin, Vec3 light_dir,
                      const std::vector<FlatNode>& flat_nodes,
                      float max_dist, int max_steps) {
    float t = 0.01f;
    for (int i = 0; i < max_steps; i++) {
        Vec3 p = {origin.x + light_dir.x * t, origin.y + light_dir.y * t, origin.z + light_dir.z * t};
        auto res = evalScene(rih, p, flat_nodes);
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
// Main render entry point
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

    // Build light list: parse lights or use default directional
    auto getLights = [&]() -> std::vector<Light> {
        if (!rih.lights.empty()) return rih.lights;
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

    // Shading with multiple lights
    auto shadeWithLights = [&](Vec3 point, Vec3 normal, Vec3 view_dir, const Material& mat) -> Rgba {
        float total_r = 0, total_g = 0, total_b = 0;
        float mr = mat.tensor[4].constant;
        float mg = mat.tensor[5].constant;
        float mb = mat.tensor[6].constant;
        float ma = mat.tensor[7].constant;

        for (auto& L : lights) {
            Vec3 l;
            float atten = 1.0f;
            if (L.type == LightType::POINT) {
                l.x = L.position.x - point.x;
                l.y = L.position.y - point.y;
                l.z = L.position.z - point.z;
                float dist = sqrtf(l.x * l.x + l.y * l.y + l.z * l.z);
                if (dist > 0.001f) { l.x /= dist; l.y /= dist; l.z /= dist; } else continue;
                atten = 1.0f / (1.0f + L.falloff * dist * dist);
            } else {
                l = L.direction;
            }
            float shadow = 1.0f;
            if (L.type == LightType::DIRECTIONAL && (point.x != 0 || point.y != 0 || point.z != 0)) {
                Vec3 so = {point.x + normal.x * 0.01f, point.y + normal.y * 0.01f, point.z + normal.z * 0.01f};
                if (shadowRay(rih, so, l, flat_nodes, 5.0f, 32))
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
            total_r += lr; total_g += lg; total_b += lb;
        }
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

            auto hit = rayMarch(rih, ro, rd, flat_nodes, 0.01f, 50.0f, 128);

            float vol_density = 0, vol_er = 0, vol_eg = 0, vol_eb = 0;
            bool has_vol = rayMarchVol(rih, ro, rd, flat_nodes, 0.01f, 50.0f, 64,
                                       vol_density, vol_er, vol_eg, vol_eb);
            float transmittance = expf(-vol_density);

            if (has_vol && transmittance < 0.01f) {
                auto clamp = [](float v) -> uint8_t {
                    return (uint8_t)fminf(255, fmaxf(0, v * 255));
                };
                pixels[py * width + px] = {clamp(vol_er), clamp(vol_eg), clamp(vol_eb), 255};
            } else if (hit.hit || has_vol) {
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
                Rgba solid = shadeWithLights(hit.hit ? hit.point : Vec3{}, hit.normal, view_dir, *mat);
                auto clamp = [](float v) -> uint8_t {
                    return (uint8_t)fminf(255, fmaxf(0, v * 255));
                };
                if (has_vol) {
                    float r = vol_er + transmittance * (solid.r / 255.0f);
                    float g = vol_eg + transmittance * (solid.g / 255.0f);
                    float b = vol_eb + transmittance * (solid.b / 255.0f);
                    pixels[py * width + px] = {clamp(r), clamp(g), clamp(b), 255};
                } else {
                    pixels[py * width + px] = solid;
                }
            } else {
                auto clamp = [](float v) -> uint8_t {
                    return (uint8_t)fminf(255, fmaxf(0, v * 255));
                };
                pixels[py * width + px] = {clamp(bg.r), clamp(bg.g), clamp(bg.b), 255};
            }
        }
    }

    return pixels;
}
