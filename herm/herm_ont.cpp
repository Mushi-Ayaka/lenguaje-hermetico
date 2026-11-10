#include "herm_ont.h"
#include "herm_eval.h"
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <unordered_map>
#include <stack>
#include <sstream>
#include <stdexcept>
#include <cctype>

namespace herm {

// ============================================================================
// Intermediate struct: flattened node ready for BVH + .ont
// ============================================================================
struct OntFlatNode {
    float local_transform[16]; // column-major 4x4 (world → local)
    float bbox_min[4];
    float bbox_max[4];
    float d_min;               // minimum guaranteed safe step inside this AABB (world space)
    std::vector<uint8_t> bytecode; // raw [op, const_f32_as_bytes, op, ...]
    uint32_t material_id;
    uint8_t  mode; // 0=solid, 1=volume
    uint32_t id;   // original node id for debugging
    const std::vector<SdfNode>* sdf_nodes = nullptr; // SDF tree for d_min eval
    uint32_t sdf_root = 0;
};

// ============================================================================
// Bytecode builder — emits raw [u8 opcode, f32 constant] × 5 bytes
// ============================================================================
struct BytecodeBuilder {
    std::vector<uint8_t> bc;

    void emit(uint8_t op, float c = 0.0f) {
        bc.push_back(op);
        const uint8_t* p = (const uint8_t*)&c;
        bc.insert(bc.end(), p, p + 4);
    }
};

// Forward: recursive SDF tree → bytecode
static void sdfTreeToBytecode(const std::vector<SdfNode>& nodes, u32 idx, BytecodeBuilder& bb);

static const char* sdfTypeName(SdfType t) {
    switch (t) {
        case SdfType::SPHERE:      return "sphere";
        case SdfType::BOX:         return "box";
        case SdfType::CYLINDER:    return "cylinder";
        case SdfType::CAPSULE:     return "capsule";
        case SdfType::TORUS:       return "torus";
        case SdfType::PLANE:       return "plane";
        case SdfType::CONE:        return "cone";
        case SdfType::ROUNDED_BOX: return "rounded_box";
        case SdfType::OCTAHEDRON:  return "octahedron";
        case SdfType::TESSERACT:   return "tesseract";
        case SdfType::SPHERE4D:    return "sphere4d";
        case SdfType::CUSTOM:      return "custom";
        default:                   return "sdf";
    }
}

static std::string lineSuffix(int line) {
    return (line > 0) ? " (linea " + std::to_string(line) + ")" : "";
}

// Numero de parametros que el bytecode ONT usa de verdad por primitiva.
static int ontParamCount(SdfType t) {
    switch (t) {
        case SdfType::SPHERE:      return 1;
        case SdfType::BOX:         return 3;
        case SdfType::CYLINDER:    return 2;
        case SdfType::CAPSULE:     return 1; // el bytecode solo usa el radio
        case SdfType::TORUS:       return 2;
        case SdfType::PLANE:       return 1;
        case SdfType::CONE:        return 2;
        case SdfType::ROUNDED_BOX: return 4;
        case SdfType::OCTAHEDRON:  return 1;
        case SdfType::TESSERACT:   return 1;
        case SdfType::SPHERE4D:    return 1;
        case SdfType::CUSTOM:      return 1;
        default:                   return 0;
    }
}

// 2.0c.3b (DL-36): adaptador BytecodeBuilder -> OntExprSink.
struct BytecodeExprSink : OntExprSink {
    BytecodeBuilder& bb;
    explicit BytecodeExprSink(BytecodeBuilder& b) : bb(b) {}
    void emitOp(uint8_t op) override { bb.emit(op); }
    void emitConst(float v) override { bb.emit(ONT_CONST, v); }
};

// Emite un parametro SDF: constante (ONT_CONST) o expresion compilada.
// El error lleva primitiva, indice, expresion y linea.
static void emitParamExpr(const SdfNode& n, int i, BytecodeBuilder& bb) {
    const Expr& p = n.params[i];
    if (!p.is_expr) {
        bb.emit(ONT_CONST, p.constant);
        return;
    }
    try {
        BytecodeExprSink sink(bb);
        compileOntExpression(p.expression, sink);
    } catch (const std::exception& ex) {
        throw std::runtime_error(
            "el parametro " + std::to_string(i) + " de '" +
            sdfTypeName(n.type) + "' no se puede compilar a bytecode ('" +
            p.expression + "'): " + ex.what() + lineSuffix(p.line));
    }
}

// Igual que emitParamExpr, pero dividiendo entre `divisor`. Para constantes
// conserva el plegado previo (h/2) y no cambia el bytecode v1.
static void emitParamDiv(const SdfNode& n, int i, float divisor,
                         BytecodeBuilder& bb) {
    const Expr& p = n.params[i];
    if (!p.is_expr) {
        bb.emit(ONT_CONST, p.constant / divisor);
        return;
    }
    emitParamExpr(n, i, bb);
    bb.emit(ONT_CONST, divisor);
    bb.emit(ONT_DIV);
}

static void sdfPrimToBytecode(const SdfNode& n, BytecodeBuilder& bb) {
    // CUSTOM no tiene formula .ont: los presets (terrain/rock/...) no son
    // expresiones y DL-31 lo rechaza.
    if (n.type == SdfType::CUSTOM) {
        std::string msg = "el SDF 'custom' no esta soportado por el bytecode ONT";
        if (n.params[0].is_expr) msg += " ('" + n.params[0].expression + "')";
        msg += lineSuffix(n.params[0].line);
        throw std::runtime_error(msg);
    }

    // Un parametro que la formula .ont no usa se descartaria en silencio.
    const int used = ontParamCount(n.type);
    for (int i = used; i < 4; i++) {
        if (n.params[i].is_expr) {
            throw std::runtime_error(
                "el parametro " + std::to_string(i) + " de '" +
                sdfTypeName(n.type) + "' es una expresion ('" +
                n.params[i].expression + "') que la formula .ont no usa; "
                "se descartaria en silencio" + lineSuffix(n.params[i].line));
        }
    }

    switch (n.type) {
        case SdfType::SPHERE: {
            // sqrt(x*x + y*y + z*z) - r
            bb.emit(ONT_VAR_X); bb.emit(ONT_VAR_X); bb.emit(ONT_MUL);
            bb.emit(ONT_VAR_Y); bb.emit(ONT_VAR_Y); bb.emit(ONT_MUL); bb.emit(ONT_ADD);
            bb.emit(ONT_VAR_Z); bb.emit(ONT_VAR_Z); bb.emit(ONT_MUL); bb.emit(ONT_ADD);
            bb.emit(ONT_SQRT);
            emitParamExpr(n, 0, bb); bb.emit(ONT_SUB);
            return;
        }
        case SdfType::BOX: {
            // max(|x|-sx, max(|y|-sy, |z|-sz))
            bb.emit(ONT_VAR_X); bb.emit(ONT_ABS); emitParamExpr(n, 0, bb); bb.emit(ONT_SUB);
            bb.emit(ONT_VAR_Y); bb.emit(ONT_ABS); emitParamExpr(n, 1, bb); bb.emit(ONT_SUB); bb.emit(ONT_MAX);
            bb.emit(ONT_VAR_Z); bb.emit(ONT_ABS); emitParamExpr(n, 2, bb); bb.emit(ONT_SUB); bb.emit(ONT_MAX);
            return;
        }
        case SdfType::CYLINDER: {
            // max(sqrt(x*x+z*z)-r, |y|-h/2)
            bb.emit(ONT_VAR_X); bb.emit(ONT_VAR_X); bb.emit(ONT_MUL);
            bb.emit(ONT_VAR_Z); bb.emit(ONT_VAR_Z); bb.emit(ONT_MUL); bb.emit(ONT_ADD);
            bb.emit(ONT_SQRT); emitParamExpr(n, 0, bb); bb.emit(ONT_SUB);
            bb.emit(ONT_VAR_Y); bb.emit(ONT_ABS); emitParamDiv(n, 1, 2.0f, bb); bb.emit(ONT_SUB);
            bb.emit(ONT_MAX);
            return;
        }
        case SdfType::CAPSULE: {
            // sqrt(x*x+y*y+z*z)-r
            bb.emit(ONT_VAR_X); bb.emit(ONT_VAR_X); bb.emit(ONT_MUL);
            bb.emit(ONT_VAR_Y); bb.emit(ONT_VAR_Y); bb.emit(ONT_MUL); bb.emit(ONT_ADD);
            bb.emit(ONT_VAR_Z); bb.emit(ONT_VAR_Z); bb.emit(ONT_MUL); bb.emit(ONT_ADD);
            bb.emit(ONT_SQRT); emitParamExpr(n, 0, bb); bb.emit(ONT_SUB);
            return;
        }
        case SdfType::TORUS: {
            // sqrt((sqrt(x*x+z*z)-R)^2 + y^2) - r
            // Need to compute q = sqrt(x*x+z*z)-R twice (no DUP opcode)
            auto pushQ = [&]() {
                bb.emit(ONT_VAR_X); bb.emit(ONT_VAR_X); bb.emit(ONT_MUL);
                bb.emit(ONT_VAR_Z); bb.emit(ONT_VAR_Z); bb.emit(ONT_MUL); bb.emit(ONT_ADD);
                bb.emit(ONT_SQRT); emitParamExpr(n, 0, bb); bb.emit(ONT_SUB);
            };
            pushQ(); pushQ(); bb.emit(ONT_MUL); // q*q
            bb.emit(ONT_VAR_Y); bb.emit(ONT_VAR_Y); bb.emit(ONT_MUL); bb.emit(ONT_ADD); // q*q + y*y
            bb.emit(ONT_SQRT); emitParamExpr(n, 1, bb); bb.emit(ONT_SUB);
            return;
        }
        case SdfType::PLANE: {
            bb.emit(ONT_VAR_Y); emitParamExpr(n, 0, bb); bb.emit(ONT_ADD);
            return;
        }
        case SdfType::CONE: {
            // max(sqrt(x*x+z*z)-r*(1-y/h), y)
            bb.emit(ONT_VAR_X); bb.emit(ONT_VAR_X); bb.emit(ONT_MUL);
            bb.emit(ONT_VAR_Z); bb.emit(ONT_VAR_Z); bb.emit(ONT_MUL); bb.emit(ONT_ADD);
            bb.emit(ONT_SQRT);
            bb.emit(ONT_CONST, 1.0f); bb.emit(ONT_VAR_Y); emitParamExpr(n, 1, bb); bb.emit(ONT_DIV); bb.emit(ONT_SUB);
            emitParamExpr(n, 0, bb); bb.emit(ONT_MUL);
            bb.emit(ONT_SUB);
            bb.emit(ONT_VAR_Y); bb.emit(ONT_MAX);
            return;
        }
        case SdfType::ROUNDED_BOX: {
            // max(|x|-sx, max(|y|-sy, |z|-sz)) - rr
            bb.emit(ONT_VAR_X); bb.emit(ONT_ABS); emitParamExpr(n, 0, bb); bb.emit(ONT_SUB);
            bb.emit(ONT_VAR_Y); bb.emit(ONT_ABS); emitParamExpr(n, 1, bb); bb.emit(ONT_SUB); bb.emit(ONT_MAX);
            bb.emit(ONT_VAR_Z); bb.emit(ONT_ABS); emitParamExpr(n, 2, bb); bb.emit(ONT_SUB); bb.emit(ONT_MAX);
            emitParamExpr(n, 3, bb); bb.emit(ONT_SUB);
            return;
        }
        case SdfType::OCTAHEDRON: {
            // (|x|+|y|+|z|-s) / 1.73205
            bb.emit(ONT_VAR_X); bb.emit(ONT_ABS);
            bb.emit(ONT_VAR_Y); bb.emit(ONT_ABS); bb.emit(ONT_ADD);
            bb.emit(ONT_VAR_Z); bb.emit(ONT_ABS); bb.emit(ONT_ADD);
            emitParamExpr(n, 0, bb); bb.emit(ONT_SUB);
            bb.emit(ONT_CONST, 1.73205f); bb.emit(ONT_DIV);
            return;
        }
        case SdfType::TESSERACT: {
            // max(|x|-s, max(|y|-s, max(|z|-s, |w|-s)))
            bb.emit(ONT_VAR_X); bb.emit(ONT_ABS); emitParamExpr(n, 0, bb); bb.emit(ONT_SUB);
            bb.emit(ONT_VAR_Y); bb.emit(ONT_ABS); emitParamExpr(n, 0, bb); bb.emit(ONT_SUB); bb.emit(ONT_MAX);
            bb.emit(ONT_VAR_Z); bb.emit(ONT_ABS); emitParamExpr(n, 0, bb); bb.emit(ONT_SUB); bb.emit(ONT_MAX);
            bb.emit(ONT_VAR_W); bb.emit(ONT_ABS); emitParamExpr(n, 0, bb); bb.emit(ONT_SUB); bb.emit(ONT_MAX);
            return;
        }
        case SdfType::SPHERE4D: {
            // sqrt(x*x+y*y+z*z+w*w) - r
            bb.emit(ONT_VAR_X); bb.emit(ONT_VAR_X); bb.emit(ONT_MUL);
            bb.emit(ONT_VAR_Y); bb.emit(ONT_VAR_Y); bb.emit(ONT_MUL); bb.emit(ONT_ADD);
            bb.emit(ONT_VAR_Z); bb.emit(ONT_VAR_Z); bb.emit(ONT_MUL); bb.emit(ONT_ADD);
            bb.emit(ONT_VAR_W); bb.emit(ONT_VAR_W); bb.emit(ONT_MUL); bb.emit(ONT_ADD);
            bb.emit(ONT_SQRT); emitParamExpr(n, 0, bb); bb.emit(ONT_SUB);
            return;
        }
        default:
            throw std::runtime_error("tipo de SDF desconocido en el bytecode ONT");
    }
}

static void sdfOpToBytecode(const SdfNode& n, const std::vector<SdfNode>& nodes, BytecodeBuilder& bb) {
    bool has_a = n.child_a.has_value();
    bool has_b = n.child_b.has_value();

    switch (n.type) {
        case SdfType::UNION: {
            if (has_a) sdfTreeToBytecode(nodes, *n.child_a, bb);
            else bb.emit(ONT_CONST, 1e9f);
            if (has_b) sdfTreeToBytecode(nodes, *n.child_b, bb);
            else bb.emit(ONT_CONST, 1e9f);
            bb.emit(ONT_MIN);
            return;
        }
        case SdfType::SUBTRACT: {
            if (has_a) sdfTreeToBytecode(nodes, *n.child_a, bb);
            else bb.emit(ONT_CONST, 1e9f);
            if (has_b) { sdfTreeToBytecode(nodes, *n.child_b, bb); bb.emit(ONT_NEG); }
            else bb.emit(ONT_CONST, 1e9f);
            bb.emit(ONT_MAX);
            return;
        }
        case SdfType::INTERSECT:
        case SdfType::SMOOTH_INTERSECT: {
            if (has_a) sdfTreeToBytecode(nodes, *n.child_a, bb);
            else bb.emit(ONT_CONST, 1e9f);
            if (has_b) sdfTreeToBytecode(nodes, *n.child_b, bb);
            else bb.emit(ONT_CONST, 1e9f);
            bb.emit(ONT_MAX);
            return;
        }
        case SdfType::SMOOTH_UNION: {
            // Without DUP opcode, (a+b - sqrt((a-b)^2 + k))/2 requires
            // computing child bytecodes 4×, which is wasteful.
            // Fall back to plain union (min) for smooth_union.
            if (has_a) sdfTreeToBytecode(nodes, *n.child_a, bb);
            else bb.emit(ONT_CONST, 1e9f);
            if (has_b) sdfTreeToBytecode(nodes, *n.child_b, bb);
            else bb.emit(ONT_CONST, 1e9f);
            bb.emit(ONT_MIN);
            return;
        }
        case SdfType::SMOOTH_SUBTRACT: {
            // Fall back to sharp subtract
            if (has_a) sdfTreeToBytecode(nodes, *n.child_a, bb);
            else bb.emit(ONT_CONST, 1e9f);
            if (has_b) { sdfTreeToBytecode(nodes, *n.child_b, bb); bb.emit(ONT_NEG); }
            else bb.emit(ONT_CONST, 1e9f);
            bb.emit(ONT_MAX);
            return;
        }
        case SdfType::REPEAT:
        case SdfType::TWIST:
        case SdfType::ELONGATE:
        case SdfType::DISPLACE:
        default: {
            // Pass-through: evaluate child A (or B if no A)
            if (has_a) sdfTreeToBytecode(nodes, *n.child_a, bb);
            else if (has_b) sdfTreeToBytecode(nodes, *n.child_b, bb);
            else bb.emit(ONT_CONST, 1e9f);
            return;
        }
    }
}

static void sdfTreeToBytecode(const std::vector<SdfNode>& nodes, u32 idx, BytecodeBuilder& bb) {
    if (idx >= nodes.size()) {
        bb.emit(ONT_CONST, 1e9f);
        return;
    }
    const auto& n = nodes[idx];
    if (n.type < SdfType::UNION)
        sdfPrimToBytecode(n, bb);
    else
        sdfOpToBytecode(n, nodes, bb);
}

static std::vector<uint8_t> sdfTreeToBytecode(const std::vector<SdfNode>& nodes, u32 idx) {
    BytecodeBuilder bb;
    sdfTreeToBytecode(nodes, idx, bb);
    bb.emit(ONT_END);
    return bb.bc;
}

// ============================================================================
// SDF tree → expression string converter (only for debugging/reference now)
// ============================================================================

static std::string paramToExpr(const Expr& p) {
    if (p.is_expr) {
        // Already a mathematical expression string
        return "(" + p.expression + ")";
    }
    return "(" + std::to_string(p.constant) + ")";
}

static std::string sdfTreeToExpr(const std::vector<SdfNode>& nodes, u32 idx);

static std::string sdfPrimToExpr(const SdfNode& n) {
    switch (n.type) {
        case SdfType::SPHERE:
            return "(sqrt(x*x+y*y+z*z)-" + paramToExpr(n.params[0]) + ")";
        case SdfType::BOX: {
            std::string sx = paramToExpr(n.params[0]);
            std::string sy = paramToExpr(n.params[1]);
            std::string sz = paramToExpr(n.params[2]);
            return "(max(abs(x)-" + sx + ",max(abs(y)-" + sy + ",abs(z)-" + sz + ")))";
        }
        case SdfType::CYLINDER: {
            std::string r = paramToExpr(n.params[0]);
            std::string h = paramToExpr(n.params[1]);
            return "(max(sqrt(x*x+z*z)-" + r + ",abs(y)-" + h + "/2))";
        }
        case SdfType::CAPSULE: {
            std::string r = paramToExpr(n.params[0]);
            // Capsule along Y axis, centered at origin, length 2*h
            // Approximation: just sphere for now, or full capsule formula
            return "(sqrt(x*x+y*y+z*z)-" + r + ")";
        }
        case SdfType::TORUS: {
            std::string R = paramToExpr(n.params[0]);
            std::string r = paramToExpr(n.params[1]);
            return "(sqrt((sqrt(x*x+z*z)-" + R + ")*(sqrt(x*x+z*z)-" + R + ")+y*y)-" + r + ")";
        }
        case SdfType::PLANE:
            return "(y+" + paramToExpr(n.params[0]) + ")";
        case SdfType::CONE: {
            std::string r = paramToExpr(n.params[0]);
            std::string h = paramToExpr(n.params[1]);
            // Approximation: q = sqrt(x*x+z*z), d = max(q - r*(1-y/h), h-y, -y)
            // Simplified to a cone along Y
            return "(max(sqrt(x*x+z*z)-" + r + "*(1-y/" + h + "),y))";
        }
        case SdfType::ROUNDED_BOX: {
            std::string sx = paramToExpr(n.params[0]);
            std::string sy = paramToExpr(n.params[1]);
            std::string sz = paramToExpr(n.params[2]);
            std::string rr = paramToExpr(n.params[3]);
            return "(max(abs(x)-" + sx + ",max(abs(y)-" + sy + ",abs(z)-" + sz + "))-" + rr + ")";
        }
        case SdfType::OCTAHEDRON: {
            std::string s = paramToExpr(n.params[0]);
            return "((abs(x)+abs(y)+abs(z)-" + s + ")/1.73205)";
        }
        case SdfType::TESSERACT: {
            std::string s = paramToExpr(n.params[0]);
            return "(max(abs(x)-" + s + ",max(abs(y)-" + s + ",max(abs(z)-" + s + ",abs(w)-" + s + "))))";
        }
        case SdfType::SPHERE4D: {
            std::string r = paramToExpr(n.params[0]);
            return "(sqrt(x*x+y*y+z*z+w*w)-" + r + ")";
        }
        case SdfType::CUSTOM: {
            if (!n.params[0].is_expr) return "0.0";
            const std::string& e = n.params[0].expression;
            if (e == "terrain") {
                // FBM terrain via sin/cos layers
                return "y-(sin(x*0.3)*sin(z*0.2)*1.5+cos(x*0.1+z*0.15)*2.0+sin(x*0.05+z*0.07)*3.0)";
            }
            if (e == "rock") {
                // Sphere with surface perturbation
                return "sqrt(x*x+y*y+z*z)-(0.5+sin(x*5.0)*sin(y*3.0)*sin(z*4.0)*0.3)";
            }
            if (e == "tree") {
                // Cone: radius 0.3 at base, height 3
                return "max(sqrt(x*x+z*z)-(0.3-abs(y)*0.1),abs(y)-3.0)";
            }
            if (e == "cloud") {
                // Soft sphere with 3D noise-like perturbation
                return "sqrt(x*x+y*y+z*z)-2.0+sin(x*8.0+y*5.0)*0.3+sin(y*7.0+z*6.0)*0.3+sin(z*9.0+x*4.0)*0.3";
            }
            if (e == "sphere4d") {
                // 4D sphere for w-axis animation
                return "sqrt(x*x+y*y+z*z+w*w)-1.0";
            }
            if (e == "water") {
                // Horizontal plane with sine waves at y=0
                return "y+sin(x*2.0+w*0.5)*0.03+cos(z*2.5+w*0.7)*0.03";
            }
            if (e == "cliff") {
                // Vertical wall with rough surface
                return "max(-y-0.5,abs(z)-2.0+sin(x*3.0)*0.3+cos(y*2.0)*0.2)";
            }
            if (e == "boulder") {
                // Sphere with stronger perturbation
                return "sqrt(x*x+y*y+z*z)-(0.8+sin(x*4.0+y*2.0)*sin(z*3.0+x*5.0)*0.4+cos(y*6.0+z*3.0)*0.2)";
            }
            if (e == "bush") {
                // Rounded clump
                return "sqrt(x*x+y*y+z*z)-(0.4+sin(x*6.0+z*5.0)*0.15+cos(y*4.0)*0.1)";
            }
            return e;
        }
        default:
            return "1e9";
    }
}

static std::string sdfOpToExpr(const SdfNode& n, const std::vector<SdfNode>& nodes) {
    std::string a = n.child_a ? sdfTreeToExpr(nodes, *n.child_a) : "1e9";
    std::string b = n.child_b ? sdfTreeToExpr(nodes, *n.child_b) : "1e9";
    switch (n.type) {
        case SdfType::UNION:            return "min(" + a + "," + b + ")";
        case SdfType::SUBTRACT:         return "max(" + a + ",-(" + b + "))";
        case SdfType::INTERSECT:        return "max(" + a + "," + b + ")";
        case SdfType::SMOOTH_UNION: {
            std::string k = n.k ? std::to_string(*n.k) : "0.5";
            return "(" + a + "+" + b + "-sqrt((" + a + "-" + b + ")*(" + a + "-" + b + ")+" + k + "))/2";
        }
        case SdfType::SMOOTH_SUBTRACT:  return "max(" + a + ",-(" + b + "))"; // fallback
        case SdfType::SMOOTH_INTERSECT: return "max(" + a + "," + b + ")";    // fallback
        case SdfType::REPEAT: {
            std::string sp = n.spacing ? std::to_string(*n.spacing) : "1.0";
            return "(" + a + ")"; // repeat via domain mod not supported in bytecode yet
        }
        case SdfType::TWIST:
        case SdfType::ELONGATE:
            return a; // fallthrough
        case SdfType::DISPLACE: {
            std::string disp = n.displace_func ? *n.displace_func : "0";
            return "(" + a + "+(" + disp + "))";
        }
        default:
            return "(" + a + ")";
    }
}

static std::string sdfTreeToExpr(const std::vector<SdfNode>& nodes, u32 idx) {
    if (idx >= nodes.size()) return "1e9";
    const auto& n = nodes[idx];
    if (n.type < SdfType::UNION)
        return sdfPrimToExpr(n);
    if (n.type >= SdfType::UNION)
        return sdfOpToExpr(n, nodes);
    return "1e9";
}

// ============================================================================
// Forward: evaluate SDF at given point (for AABB + d_min computation)
// Uses Herm's rendering eval
// ============================================================================
extern float evalSdfTree(const std::vector<SdfNode>& nodes, u32 idx, Vec3 p, float w);

// Forward declarations for AABB helpers (used by computeNodeAabbRecursive)
static void aabbUnion(const float a_min[4], const float a_max[4],
                       const float b_min[4], const float b_max[4],
                       float out_min[4], float out_max[4]);

// 2.0c.3b (DL-36): un parametro SDF con expresion puede mover la superficie
// a cualquier punto. El AABB deja de ser calculable y d_min debe ser 0.
static bool sdfTreeHasExprParam(const std::vector<SdfNode>& nodes, u32 idx) {
    if (idx >= nodes.size()) return false;
    const auto& n = nodes[idx];
    if (n.type < SdfType::UNION) {
        int used = ontParamCount(n.type);
        for (int i = 0; i < used && i < 4; i++)
            if (n.params[i].is_expr) return true;
        return false;
    }
    if (n.child_a && sdfTreeHasExprParam(nodes, *n.child_a)) return true;
    if (n.child_b && sdfTreeHasExprParam(nodes, *n.child_b)) return true;
    return false;
}

// ============================================================================
// Compute AABB for an SDF tree recursively
// ============================================================================
static void computeNodeAabbRecursive(const std::vector<SdfNode>& nodes, u32 idx,
                                      float bmin[4], float bmax[4],
                                      float scene_scale = 5.0f) {
    // Default: empty AABB (min > max) — signals "no geometry" or empty node
    bmin[0] = bmin[1] = bmin[2] = bmin[3] = 1e9f;
    bmax[0] = bmax[1] = bmax[2] = bmax[3] = -1e9f;

    if (idx >= nodes.size()) return;

    // 2.0c.3b (DL-36): expresion -> AABB conservador e infinito practico.
    if (sdfTreeHasExprParam(nodes, idx)) {
        bmin[0] = bmin[1] = bmin[2] = -1e9f; bmin[3] = 0.0f;
        bmax[0] = bmax[1] = bmax[2] =  1e9f; bmax[3] = 0.0f;
        return;
    }

    const auto& node = nodes[idx];
    // Initialize local AABB to reasonable default BEFORE switch
    // so that any case that skips setting some axes still has valid values
    float local_bmin[4] = {-scene_scale, -scene_scale, -scene_scale, 0};
    float local_bmax[4] = {scene_scale,  scene_scale,  scene_scale,  0};
    float child_bmin[4], child_bmax[4];
    float cbmin[4], cbmax[4];

    auto markAabb = [&](float mn, float mx, int axis) {
        if (mn < local_bmin[axis]) local_bmin[axis] = mn;
        if (mx > local_bmax[axis]) local_bmax[axis] = mx;
    };

    switch (node.type) {
        // ====================================================================
        // PRIMITIVE TYPES — compute AABB from parameters
        // ====================================================================
        case SdfType::SPHERE: {
            float r = node.params[0].is_expr ? 1.0f : node.params[0].constant;
            if (r <= 0) r = 1.0f;
            local_bmin[0] = -r; local_bmax[0] = r;
            local_bmin[1] = -r; local_bmax[1] = r;
            local_bmin[2] = -r; local_bmax[2] = r;
            local_bmin[3] = 0;  local_bmax[3] = 0;
            break;
        }
        case SdfType::BOX: {
            float sx = node.params[0].is_expr ? 1.0f : node.params[0].constant;
            float sy = node.params[1].is_expr ? 1.0f : node.params[1].constant;
            float sz = node.params[2].is_expr ? 1.0f : node.params[2].constant;
            local_bmin[0] = -sx; local_bmax[0] = sx;
            local_bmin[1] = -sy; local_bmax[1] = sy;
            local_bmin[2] = -sz; local_bmax[2] = sz;
            local_bmin[3] = 0;   local_bmax[3] = 0;
            break;
        }
        case SdfType::PLANE: {
            // Plane is infinite — use scene_scale as conservative bound
            local_bmin[0] = -scene_scale; local_bmax[0] = scene_scale;
            local_bmin[1] = -scene_scale; local_bmax[1] = scene_scale;
            local_bmin[2] = -scene_scale; local_bmax[2] = scene_scale;
            local_bmin[3] = 0;            local_bmax[3] = 0;
            break;
        }
        case SdfType::CYLINDER: {
            float r = node.params[0].is_expr ? 1.0f : node.params[0].constant;
            float h = node.params[1].is_expr ? 1.0f : node.params[1].constant;
            if (r <= 0) r = 1.0f;
            if (h <= 0) h = 1.0f;
            local_bmin[0] = -r;       local_bmax[0] = r;
            local_bmin[1] = -h/2.0f;  local_bmax[1] = h/2.0f;
            local_bmin[2] = -r;       local_bmax[2] = r;
            local_bmin[3] = 0;        local_bmax[3] = 0;
            break;
        }
        case SdfType::TORUS: {
            float R = node.params[0].is_expr ? 1.0f : node.params[0].constant;
            float r = node.params[1].is_expr ? 0.2f : node.params[1].constant;
            local_bmin[0] = -(R+r); local_bmax[0] = R+r;
            local_bmin[1] = -r;     local_bmax[1] = r;
            local_bmin[2] = -(R+r); local_bmax[2] = R+r;
            local_bmin[3] = 0;      local_bmax[3] = 0;
            break;
        }
        case SdfType::CONE: {
            float r = node.params[0].is_expr ? 1.0f : node.params[0].constant;
            float h = node.params[1].is_expr ? 1.0f : node.params[1].constant;
            local_bmin[0] = -r; local_bmax[0] = r;
            local_bmin[1] = 0;  local_bmax[1] = h;
            local_bmin[2] = -r; local_bmax[2] = r;
            local_bmin[3] = 0;  local_bmax[3] = 0;
            break;
        }
        case SdfType::CAPSULE: {
            float r = node.params[0].is_expr ? 1.0f : node.params[0].constant;
            float h = node.params[1].is_expr ? 1.0f : node.params[1].constant;
            local_bmin[0] = -r;       local_bmax[0] = r;
            local_bmin[1] = -(h+r);   local_bmax[1] = h+r;
            local_bmin[2] = -r;       local_bmax[2] = r;
            local_bmin[3] = 0;        local_bmax[3] = 0;
            break;
        }
        case SdfType::ROUNDED_BOX: {
            float sx = node.params[0].is_expr ? 1.0f : node.params[0].constant;
            float sy = node.params[1].is_expr ? 1.0f : node.params[1].constant;
            float sz = node.params[2].is_expr ? 1.0f : node.params[2].constant;
            float rr = node.params[3].is_expr ? 0.1f : node.params[3].constant;
            local_bmin[0] = -(sx+rr); local_bmax[0] = sx+rr;
            local_bmin[1] = -(sy+rr); local_bmax[1] = sy+rr;
            local_bmin[2] = -(sz+rr); local_bmax[2] = sz+rr;
            local_bmin[3] = 0;        local_bmax[3] = 0;
            break;
        }
        case SdfType::OCTAHEDRON: {
            float s = node.params[0].is_expr ? 1.0f : node.params[0].constant;
            local_bmin[0] = -s; local_bmax[0] = s;
            local_bmin[1] = -s; local_bmax[1] = s;
            local_bmin[2] = -s; local_bmax[2] = s;
            local_bmin[3] = 0;  local_bmax[3] = 0;
            break;
        }
        case SdfType::TESSERACT: {
            float s = node.params[0].is_expr ? 1.0f : node.params[0].constant;
            local_bmin[0] = -s; local_bmax[0] = s;
            local_bmin[1] = -s; local_bmax[1] = s;
            local_bmin[2] = -s; local_bmax[2] = s;
            local_bmin[3] = -s; local_bmax[3] = s;
            break;
        }
        case SdfType::SPHERE4D: {
            float r = node.params[0].is_expr ? 1.0f : node.params[0].constant;
            if (r <= 0) r = 1.0f;
            local_bmin[0] = -r; local_bmax[0] = r;
            local_bmin[1] = -r; local_bmax[1] = r;
            local_bmin[2] = -r; local_bmax[2] = r;
            local_bmin[3] = 0;  local_bmax[3] = 0;
            break;
        }

        // ====================================================================
        // CUSTOM — sample-based AABB discovery with 16³ + octree refinement
        // ====================================================================
        case SdfType::CUSTOM: {
            local_bmin[0] = local_bmin[1] = local_bmin[2] = 1e9f;
            local_bmax[0] = local_bmax[1] = local_bmax[2] = -1e9f;
            bool found = false;

            // Phase 1: coarse 16³ grid over [-scene_scale, scene_scale]
            float coarse_step = scene_scale / 8.0f;  // 16 cells = 2*8
            int grid_pts = 16;
            float inv_g = 2.0f * scene_scale / (grid_pts - 1);

            // Track which coarse cells may contain surface
            // We use a flat array sized 16×16×16 bits = 512 bytes
            static constexpr int CELL_RES = 16;
            uint8_t cell_hits[CELL_RES * CELL_RES * CELL_RES] = {};

            auto sampleSdf = [&](float x, float y, float z) -> float {
                return evalSdfTree(nodes, idx, Vec3{x, y, z}, 0.0f);
            };

            // Phase 1a: coarse sampling
            float eps_surface = inv_g * 1.732f; // diagonal of a coarse cell
            for (int ix = 0; ix < CELL_RES; ix++) {
                float x = -scene_scale + ix * inv_g;
                for (int iy = 0; iy < CELL_RES; iy++) {
                    float y = -scene_scale + iy * inv_g;
                    for (int iz = 0; iz < CELL_RES; iz++) {
                        float z = -scene_scale + iz * inv_g;
                        float d = sampleSdf(x, y, z);
                        if (std::fabs(d) <= eps_surface) {
                            int ci = (ix * CELL_RES + iy) * CELL_RES + iz;
                            cell_hits[ci] = 1;
                            found = true;
                            markAabb(x - coarse_step, x + coarse_step, 0);
                            markAabb(y - coarse_step, y + coarse_step, 1);
                            markAabb(z - coarse_step, z + coarse_step, 2);
                        }
                    }
                }
            }

            // Phase 1b: also look for interior points (d < 0) with no surface nearby
            // If a coarse cell is entirely inside the object, we need to capture it
            // (d < 0 at all 8 corners but |d| < eps at none)
            for (int ix = 0; ix < CELL_RES; ix++) {
                float x = -scene_scale + ix * inv_g;
                for (int iy = 0; iy < CELL_RES; iy++) {
                    float y = -scene_scale + iy * inv_g;
                    for (int iz = 0; iz < CELL_RES; iz++) {
                        int ci = (ix * CELL_RES + iy) * CELL_RES + iz;
                        if (cell_hits[ci]) continue;
                        float z = -scene_scale + iz * inv_g;
                        float d = sampleSdf(x, y, z);
                        // Interior or exactly on surface
                        if (d <= eps_surface) {
                            cell_hits[ci] = 1;
                            found = true;
                            markAabb(x - coarse_step, x + coarse_step, 0);
                            markAabb(y - coarse_step, y + coarse_step, 1);
                            markAabb(z - coarse_step, z + coarse_step, 2);
                        }
                    }
                }
            }

            // Phase 2: refine around surface cells (octree subdivision)
            if (found) {
                float refine_step = coarse_step / 4.0f;  // 4× subdiv = 64³ effective
                for (int ix = 0; ix < CELL_RES; ix++) {
                    float x0 = -scene_scale + ix * inv_g;
                    for (int iy = 0; iy < CELL_RES; iy++) {
                        float y0 = -scene_scale + iy * inv_g;
                        for (int iz = 0; iz < CELL_RES; iz++) {
                            float z0 = -scene_scale + iz * inv_g;
                            int ci = (ix * CELL_RES + iy) * CELL_RES + iz;
                            if (!cell_hits[ci]) continue;

                            // Subdivide this cell into 4×4×4 sub-samples (4 per axis)
                            for (int sx = 0; sx < 4; sx++) {
                                float x = x0 + sx * refine_step;
                                for (int sy = 0; sy < 4; sy++) {
                                    float y = y0 + sy * refine_step;
                                    for (int sz = 0; sz < 4; sz++) {
                                        float z = z0 + sz * refine_step;
                                        float d = sampleSdf(x, y, z);
                                        if (std::fabs(d) <= refine_step * 1.732f) {
                                            markAabb(x - refine_step, x + refine_step, 0);
                                            markAabb(y - refine_step, y + refine_step, 1);
                                            markAabb(z - refine_step, z + refine_step, 2);
                                        } else if (d < 0) {
                                            // Interior point — still part of the shape
                                            markAabb(x - refine_step, x + refine_step, 0);
                                            markAabb(y - refine_step, y + refine_step, 1);
                                            markAabb(z - refine_step, z + refine_step, 2);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                // Expand AABB by a safety margin (2× refinement step)
                float margin = refine_step * 2.0f;
                local_bmin[0] -= margin; local_bmax[0] += margin;
                local_bmin[1] -= margin; local_bmax[1] += margin;
                local_bmin[2] -= margin; local_bmax[2] += margin;
            }

            if (!found) {
                // No surface found in sampling grid — fall back to conservative AABB
                local_bmin[0] = local_bmin[1] = local_bmin[2] = -scene_scale;
                local_bmax[0] = local_bmax[1] = local_bmax[2] = scene_scale;
            }
            local_bmin[3] = local_bmax[3] = 0;
            break;
        }

        // ====================================================================
        // CSG BINARY OPERATORS — recurse into children, combine AABBs
        // ====================================================================
        case SdfType::UNION:
        case SdfType::SMOOTH_UNION: {
            if (node.child_a.has_value())
                computeNodeAabbRecursive(nodes, node.child_a.value(), local_bmin, local_bmax, scene_scale);
            if (node.child_b.has_value()) {
                computeNodeAabbRecursive(nodes, node.child_b.value(), child_bmin, child_bmax, scene_scale);
                aabbUnion(local_bmin, local_bmax, child_bmin, child_bmax, local_bmin, local_bmax);
            }
            local_bmin[3] = local_bmax[3] = 0;
            break;
        }
        case SdfType::SUBTRACT:
        case SdfType::SMOOTH_SUBTRACT: {
            // Subtraction: result is always inside child A's volume
            if (node.child_a.has_value())
                computeNodeAabbRecursive(nodes, node.child_a.value(), local_bmin, local_bmax, scene_scale);
            else {
                local_bmin[0] = local_bmin[1] = local_bmin[2] = -scene_scale;
                local_bmax[0] = local_bmax[1] = local_bmax[2] = scene_scale;
            }
            local_bmin[3] = local_bmax[3] = 0;
            break;
        }
        case SdfType::INTERSECT:
        case SdfType::SMOOTH_INTERSECT: {
            // Default: empty — will be overwritten if both children exist and overlap
            local_bmin[0] = local_bmin[1] = local_bmin[2] = 1e9f;
            local_bmax[0] = local_bmax[1] = local_bmax[2] = -1e9f;
            local_bmin[3] = local_bmax[3] = 0;

            if (node.child_a.has_value() && node.child_b.has_value()) {
                computeNodeAabbRecursive(nodes, node.child_a.value(), child_bmin, child_bmax, scene_scale);
                computeNodeAabbRecursive(nodes, node.child_b.value(), cbmin, cbmax, scene_scale);
                // Intersection: overlap of both AABBs (only xyz, w is 0)
                local_bmin[0] = std::fmax(child_bmin[0], cbmin[0]);
                local_bmin[1] = std::fmax(child_bmin[1], cbmin[1]);
                local_bmin[2] = std::fmax(child_bmin[2], cbmin[2]);
                local_bmax[0] = std::fmin(child_bmax[0], cbmax[0]);
                local_bmax[1] = std::fmin(child_bmax[1], cbmax[1]);
                local_bmax[2] = std::fmin(child_bmax[2], cbmax[2]);
                // If no overlap, mark as empty (bmin > bmax)
                if (local_bmin[0] > local_bmax[0] + 1e-4f ||
                    local_bmin[1] > local_bmax[1] + 1e-4f ||
                    local_bmin[2] > local_bmax[2] + 1e-4f) {
                    local_bmin[0] = local_bmin[1] = local_bmin[2] = 1e9f;
                    local_bmax[0] = local_bmax[1] = local_bmax[2] = -1e9f;
                }
            }
            break;
        }

        // ====================================================================
        // UNARY OPERATORS — propagate child's AABB
        // ====================================================================
        case SdfType::REPEAT:
        case SdfType::TWIST:
        case SdfType::ELONGATE:
        case SdfType::DISPLACE:
        default: {
            // REPEAT: domain repetition — the AABB could extend to infinity,
            // but we propagate child's AABB as a conservative approximation
            // for one cell (no domain wrapping analysis yet)
            // TWIST/ELONGATE/DISPLACE: same conservative propagation
            if (node.child_a.has_value())
                computeNodeAabbRecursive(nodes, node.child_a.value(), local_bmin, local_bmax, scene_scale);
            else {
                // No child — propagate as unknown
                local_bmin[0] = local_bmin[1] = local_bmin[2] = -scene_scale;
                local_bmax[0] = local_bmax[1] = local_bmax[2] = scene_scale;
            }
            local_bmin[3] = local_bmax[3] = 0;
            break;
        }
    }

    // Copy result to output
    bmin[0] = local_bmin[0]; bmax[0] = local_bmax[0];
    bmin[1] = local_bmin[1]; bmax[1] = local_bmax[1];
    bmin[2] = local_bmin[2]; bmax[2] = local_bmax[2];
    bmin[3] = local_bmin[3]; bmax[3] = local_bmax[3];
}

// ============================================================================
// Transform helpers (column-major 4x4)
// ============================================================================
static void mat4Identity(float m[16]) {
    for (int i = 0; i < 16; i++) m[i] = 0;
    m[0] = m[5] = m[10] = m[15] = 1.0f;
}

static void mat4Mul(const float a[16], const float b[16], float out[16]) {
    for (int col = 0; col < 4; col++)
        for (int row = 0; row < 4; row++) {
            float sum = 0;
            for (int k = 0; k < 4; k++)
                sum += a[k * 4 + row] * b[col * 4 + k];
            out[col * 4 + row] = sum;
        }
}

static void mat4Translate(float m[16], float tx, float ty, float tz) {
    float t[16]; mat4Identity(t);
    t[12] = tx; t[13] = ty; t[14] = tz;
    float r[16]; mat4Mul(m, t, r);
    for (int i = 0; i < 16; i++) m[i] = r[i];
}

static void mat4Scale(float m[16], float sx, float sy, float sz) {
    float s[16]; mat4Identity(s);
    s[0] = sx; s[5] = sy; s[10] = sz;
    float r[16]; mat4Mul(m, s, r);
    for (int i = 0; i < 16; i++) m[i] = r[i];
}

static void mat4RotateX(float m[16], float deg) {
    float rad = deg * 3.14159265f / 180.0f;
    float c = std::cos(rad), s = std::sin(rad);
    float r[16]; mat4Identity(r);
    r[5] = c; r[6] = -s; r[9] = s; r[10] = c;
    float out[16]; mat4Mul(m, r, out);
    for (int i = 0; i < 16; i++) m[i] = out[i];
}

static void mat4RotateY(float m[16], float deg) {
    float rad = deg * 3.14159265f / 180.0f;
    float c = std::cos(rad), s = std::sin(rad);
    float r[16]; mat4Identity(r);
    r[0] = c; r[2] = s; r[8] = -s; r[10] = c;
    float out[16]; mat4Mul(m, r, out);
    for (int i = 0; i < 16; i++) m[i] = out[i];
}

static void mat4RotateZ(float m[16], float deg) {
    float rad = deg * 3.14159265f / 180.0f;
    float c = std::cos(rad), s = std::sin(rad);
    float r[16]; mat4Identity(r);
    r[0] = c; r[1] = -s; r[4] = s; r[5] = c;
    float out[16]; mat4Mul(m, r, out);
    for (int i = 0; i < 16; i++) m[i] = out[i];
}

// Inverse of a 4x4 affine column-major matrix (assumes [A|t; 0|1] structure)
static bool mat4Inverse(const float m[16], float out[16]) {
    // Compute 3x3 cofactors
    float a0 = m[5]*m[10] - m[6]*m[9];
    float a1 = m[4]*m[10] - m[6]*m[8];
    float a2 = m[4]*m[9] - m[5]*m[8];
    float a3 = m[1]*m[10] - m[2]*m[9];
    float a4 = m[0]*m[10] - m[2]*m[8];
    float a5 = m[0]*m[9] - m[1]*m[8];
    float a6 = m[1]*m[6] - m[2]*m[5];
    float a7 = m[0]*m[6] - m[2]*m[4];
    float a8 = m[0]*m[5] - m[1]*m[4];
    float det = m[0]*a0 - m[4]*a3 + m[8]*a6;
    if (std::abs(det) < 1e-12f) return false;
    float inv_det = 1.0f / det;
    // Transpose 3x3 cofactors (adjugate) * inv_det
    out[0] =  a0 * inv_det; out[4] = -a1 * inv_det; out[8] =  a2 * inv_det;
    out[1] = -a3 * inv_det; out[5] =  a4 * inv_det; out[9] = -a5 * inv_det;
    out[2] =  a6 * inv_det; out[6] = -a7 * inv_det; out[10]=  a8 * inv_det;
    out[3] = 0; out[7] = 0; out[11]= 0; out[15] = 1;
    // Translation: -A^-1 * t
    out[12] = -(out[0]*m[12] + out[4]*m[13] + out[8]*m[14]);
    out[13] = -(out[1]*m[12] + out[5]*m[13] + out[9]*m[14]);
    out[14] = -(out[2]*m[12] + out[6]*m[13] + out[10]*m[14]);
    return true;
}

// Transform local-space AABB to world space using local→world matrix (inverse of world→local)
static void aabbLocalToWorld(const float local_xform[16], float bmin[4], float bmax[4]) {
    float inv[16];
    if (!mat4Inverse(local_xform, inv)) return;
    float wmin[4] = {1e9f,1e9f,1e9f,1e9f};
    float wmax[4] = {-1e9f,-1e9f,-1e9f,-1e9f};
    for (int i = 0; i < 8; i++) {
        float lx = (i & 1) ? bmin[0] : bmax[0];
        float ly = (i & 2) ? bmin[1] : bmax[1];
        float lz = (i & 4) ? bmin[2] : bmax[2];
        float wx = inv[0]*lx + inv[4]*ly + inv[8]*lz + inv[12];
        float wy = inv[1]*lx + inv[5]*ly + inv[9]*lz + inv[13];
        float wz = inv[2]*lx + inv[6]*ly + inv[10]*lz + inv[14];
        if (wx < wmin[0]) wmin[0] = wx;
        if (wy < wmin[1]) wmin[1] = wy;
        if (wz < wmin[2]) wmin[2] = wz;
        if (wx > wmax[0]) wmax[0] = wx;
        if (wy > wmax[1]) wmax[1] = wy;
        if (wz > wmax[2]) wmax[2] = wz;
    }
    bmin[0] = wmin[0]; bmin[1] = wmin[1]; bmin[2] = wmin[2];
    bmax[0] = wmax[0]; bmax[1] = wmax[1]; bmax[2] = wmax[2];
}

// World → local transform from translate, rotate, scale
static void makeWorldToLocal(const Expr tr[3], const Expr rot[3], const Expr sc[3],
                              float out[16], float w_frame) {
    mat4Identity(out);
    // In SDF rendering, we apply: p = (p - translate) * inv(rotate) / scale
    // The matrix stored is world→local, so it's: scale^-1 * rotate^-1 * (translate^-1)
    // But we store the inverse: children are transformed to parent space
    
    // Build: first translate, then rotate, then scale
    float tx = tr[0].is_expr ? 0 : tr[0].constant;
    float ty = tr[1].is_expr ? 0 : tr[1].constant;
    float tz = tr[2].is_expr ? 0 : tr[2].constant;
    mat4Translate(out, -tx, -ty, -tz);
    
    float rx = rot[0].is_expr ? 0 : rot[0].constant;
    float ry = rot[1].is_expr ? 0 : rot[1].constant;
    float rz = rot[2].is_expr ? 0 : rot[2].constant;
    if (rx != 0) mat4RotateX(out, -rx);
    if (ry != 0) mat4RotateY(out, -ry);
    if (rz != 0) mat4RotateZ(out, -rz);
    
    float sx = sc[0].is_expr ? 1.0f : (sc[0].constant != 0 ? 1.0f / sc[0].constant : 1.0f);
    float sy = sc[1].is_expr ? 1.0f : (sc[1].constant != 0 ? 1.0f / sc[1].constant : 1.0f);
    float sz = sc[2].is_expr ? 1.0f : (sc[2].constant != 0 ? 1.0f / sc[2].constant : 1.0f);
    mat4Scale(out, sx, sy, sz);
}

// ============================================================================
// Resolve instance/group hierarchy into flat node list
// ============================================================================
static void flattenNode(const Node& node, const Rih& rih,
                         const float parent_xform[16],
                         std::vector<OntFlatNode>& out,
                         std::vector<bool>& visited,
                         float w_frame) {
    if (node.id < visited.size() && visited[node.id]) return;
    if (node.id < visited.size()) visited[node.id] = true;

    if (node.type == NodeType::GROUP) {
        for (u32 child_id : node.children) {
            // Find child node
            const Node* child = nullptr;
            for (auto& n : rih.nodes) {
                if (n.id == child_id) { child = &n; break; }
            }
            if (!child) continue;

            float child_xform[16];
            makeWorldToLocal(child->transform.translate, child->transform.rotate,
                             child->transform.scale, child_xform, w_frame);
            float combined[16];
            mat4Mul(parent_xform, child_xform, combined);

            flattenNode(*child, rih, combined, out, visited, w_frame);
        }
    } else if (node.type == NodeType::INSTANCE) {
        if (node.def_id) {
            float inst_xform[16];
            makeWorldToLocal(node.transform.translate, node.transform.rotate,
                             node.transform.scale, inst_xform, w_frame);
            float after_inst[16];
            mat4Mul(parent_xform, inst_xform, after_inst);

            const Node* def = nullptr;
            for (auto& d : rih.defs) {
                if (d.id == *node.def_id) { def = &d; break; }
            }
            if (def) {
                float def_xform[16];
                makeWorldToLocal(def->transform.translate, def->transform.rotate,
                                 def->transform.scale, def_xform, w_frame);
                float combined[16];
                mat4Mul(after_inst, def_xform, combined);
                flattenNode(*def, rih, combined, out, visited, w_frame);
            }
        }
    } else if (node.type == NodeType::SDF) {
        OntFlatNode fn;
        float node_xform[16];
        makeWorldToLocal(node.transform.translate, node.transform.rotate,
                         node.transform.scale, node_xform, w_frame);
        float combined[16];
        mat4Mul(parent_xform, node_xform, combined);
        for (int i = 0; i < 16; i++) fn.local_transform[i] = combined[i];
        fn.material_id = node.material_id.value_or(0xFFFFFFFF);
        fn.mode = (node.mode == NodeMode::VOLUME) ? 1 : 0;
        fn.id = node.id;

        // Convert SDF tree directly to bytecode opcodes
        if (!node.sdf_nodes.empty()) {
            fn.bytecode = sdfTreeToBytecode(node.sdf_nodes, 0);
        }

        // Compute local AABB via recursive SDF tree evaluation
        float local_min[4], local_max[4];
        computeNodeAabbRecursive(node.sdf_nodes, 0, local_min, local_max);

        // Compute d_min for adaptive stepping:
        // local_d_min = max(0, SDF(center) - half_diagonal)
        // Uses 1-Lipschitz property: min SDF in AABB >= SDF(center) - max_dist_from_center
        // 2.0c.3b (DL-36): con expresion, evalSdfTree lee params[i].constant
        // y da un valor espurio; d_min = 0 desactiva el paso seguro.
        if (sdfTreeHasExprParam(node.sdf_nodes, 0)) {
            fn.d_min = 0.0f;
        } else {
            float cx = (local_min[0] + local_max[0]) * 0.5f;
            float cy = (local_min[1] + local_max[1]) * 0.5f;
            float cz = (local_min[2] + local_max[2]) * 0.5f;
            float dx = local_max[0] - local_min[0];
            float dy = local_max[1] - local_min[1];
            float dz = local_max[2] - local_min[2];
            float half_diag = 0.5f * std::sqrt(dx*dx + dy*dy + dz*dz);
            float sdf_at_center = evalSdfTree(node.sdf_nodes, 0, Vec3{cx, cy, cz}, 0.0f);
            float local_d_min = std::fmax(0.0f, sdf_at_center - half_diag);
            // Convert local d_min to world space
            float scale = std::sqrt(combined[0]*combined[0] + combined[1]*combined[1] + combined[2]*combined[2]);
            fn.d_min = (scale > 1e-8f) ? (local_d_min / scale) : 0.0f;
        }

        // Copy local AABB, then transform to world space
        for (int j = 0; j < 4; j++) {
            fn.bbox_min[j] = local_min[j];
            fn.bbox_max[j] = local_max[j];
        }
        aabbLocalToWorld(combined, fn.bbox_min, fn.bbox_max);

        // Store SDF tree reference for BVH-level d_min computation
        fn.sdf_nodes = &node.sdf_nodes;
        fn.sdf_root = 0;

        out.push_back(fn);
    }
}

// ============================================================================
// BVH Node (temporary tree structure before linearization)
// ============================================================================
struct BvhBuildNode {
    float min[4], max[4];
    float d_min, L;
    uint32_t first_node;
    uint16_t node_count;
    bool is_leaf;
    BvhBuildNode* left = nullptr;
    BvhBuildNode* right = nullptr;
};

static float aabbSurfaceArea(const float min[4], const float max[4]) {
    float dx = max[0] - min[0];
    float dy = max[1] - min[1];
    float dz = max[2] - min[2];
    if (dx < 0) dx = 0;
    if (dy < 0) dy = 0;
    if (dz < 0) dz = 0;
    return 2.0f * (dx*dy + dy*dz + dz*dx);
}

static void aabbUnion(const float a_min[4], const float a_max[4],
                       const float b_min[4], const float b_max[4],
                       float out_min[4], float out_max[4]) {
    out_min[0] = std::fmin(a_min[0], b_min[0]);
    out_min[1] = std::fmin(a_min[1], b_min[1]);
    out_min[2] = std::fmin(a_min[2], b_min[2]);
    out_max[0] = std::fmax(a_max[0], b_max[0]);
    out_max[1] = std::fmax(a_max[1], b_max[1]);
    out_max[2] = std::fmax(a_max[2], b_max[2]);
    // w component is always 0 for 3D AABBs
    out_min[3] = 0;
    out_max[3] = 0;
}

// Simple median-split BVH builder
static BvhBuildNode* buildBvhRecursive(
    std::vector<OntFlatNode*>& items,
    uint32_t first, uint32_t count,
    uint32_t& node_counter,
    uint32_t max_leaf_size = 4)
{
    auto* node = new BvhBuildNode();
    node->first_node = first;
    node->node_count = (uint16_t)count;

    // Compute AABB
    if (count > 0) {
        for (int i = 0; i < 4; i++) {
            node->min[i] = items[first]->bbox_min[i];
            node->max[i] = items[first]->bbox_max[i];
        }
        for (uint32_t i = first + 1; i < first + count; i++) {
            aabbUnion(node->min, node->max,
                      items[i]->bbox_min, items[i]->bbox_max,
                      node->min, node->max);
        }
    }

    // Compute L (placeholder - always 1.0)
    node->L = 1.0f;

    if (count <= max_leaf_size) {
        node->is_leaf = true;

        // 2.0c.3b (DL-36): una hoja con expresion no se puede muestrear con
        // evalSdfTree (lee params[i].constant). d_min = 0 desactiva el paso
        // seguro de esa hoja en vez de inventar una cota.
        bool leaf_has_expr = false;
        for (uint32_t i = first; i < first + count; i++) {
            auto* fn = items[i];
            if (fn->sdf_nodes && !fn->sdf_nodes->empty() &&
                sdfTreeHasExprParam(*fn->sdf_nodes, fn->sdf_root)) {
                leaf_has_expr = true;
                break;
            }
        }
        if (leaf_has_expr) {
            node->d_min = 0.0f;
            return node;
        }

        // Compute d_min from combined leaf AABB: SDF(center) - half_diag
        // Sample all OntFlatNodes at the center of the leaf's combined AABB
        {
            float cx = (node->min[0] + node->max[0]) * 0.5f;
            float cy = (node->min[1] + node->max[1]) * 0.5f;
            float cz = (node->min[2] + node->max[2]) * 0.5f;
            float dx = node->max[0] - node->min[0];
            float dy = node->max[1] - node->min[1];
            float dz = node->max[2] - node->min[2];
            float half_diag = 0.5f * std::sqrt(dx*dx + dy*dy + dz*dz);
            float min_sdf = 1e9f;
            for (uint32_t i = first; i < first + count; i++) {
                auto* fn = items[i];
                if (!fn->sdf_nodes || fn->sdf_nodes->empty()) continue;
                float lx = fn->local_transform[0]*cx + fn->local_transform[4]*cy + fn->local_transform[8]*cz  + fn->local_transform[12];
                float ly = fn->local_transform[1]*cx + fn->local_transform[5]*cy + fn->local_transform[9]*cz  + fn->local_transform[13];
                float lz = fn->local_transform[2]*cx + fn->local_transform[6]*cy + fn->local_transform[10]*cz + fn->local_transform[14];
                float sdf_local = evalSdfTree(*fn->sdf_nodes, fn->sdf_root, Vec3{lx, ly, lz}, 0.0f);
                float scale = std::sqrt(fn->local_transform[0]*fn->local_transform[0] +
                                         fn->local_transform[1]*fn->local_transform[1] +
                                         fn->local_transform[2]*fn->local_transform[2]);
                float sdf_world = (scale > 1e-8f) ? (sdf_local / scale) : sdf_local;
                if (sdf_world < min_sdf) min_sdf = sdf_world;
            }
            node->d_min = std::fmax(0.0f, min_sdf - half_diag);
        }
        return node;
    }

    // Find split axis: longest AABB extent
    float extent[3] = {
        node->max[0] - node->min[0],
        node->max[1] - node->min[1],
        node->max[2] - node->min[2]
    };
    int axis = 0;
    if (extent[1] > extent[0]) axis = 1;
    if (extent[2] > extent[axis]) axis = 2;

    // Sort by centroid on axis (simple median split)
    auto centroid = [&](OntFlatNode* fn, int ax) -> float {
        return (fn->bbox_min[ax] + fn->bbox_max[ax]) * 0.5f;
    };

    std::sort(items.begin() + first, items.begin() + first + count,
        [&](OntFlatNode* a, OntFlatNode* b) {
            return centroid(a, axis) < centroid(b, axis);
        });

    uint32_t mid = count / 2;

    // Adjust mid to avoid degenerate splits
    if (mid == 0) mid = 1;
    if (mid == count) mid = count - 1;

    node->left = buildBvhRecursive(items, first, mid, node_counter, max_leaf_size);
    node->right = buildBvhRecursive(items, first + mid, count - mid, node_counter, max_leaf_size);
    node->is_leaf = false;

    // Recompute AABB from children (tighter)
    aabbUnion(node->left->min, node->left->max,
              node->right->min, node->right->max,
              node->min, node->max);

    node->d_min = std::fmin(node->left->d_min, node->right->d_min);
    node->L = std::fmax(node->left->L, node->right->L);

    return node;
}

// Linearize BVH tree into flat array with skip_index for stackless traversal.
// Returns the index of the first node AFTER this subtree.
static uint32_t linearizeBvh(BvhBuildNode* node,
                              std::vector<BvhNode>& flat,
                              uint32_t& idx)
{
    uint32_t my_idx = idx++;
    BvhNode bn;
    for (int i = 0; i < 4; i++) {
        bn.min[i] = node->min[i];
        bn.max[i] = node->max[i];
    }
    bn.d_min = node->d_min;
    bn.L = node->L;
    bn.first_node = node->first_node;
    bn.node_count = node->node_count;
    bn.flags = node->is_leaf ? BVH_FLAG_LEAF : 0;
    bn.skip_index = 0;

    flat.push_back(bn);

    uint32_t end_idx;
    if (!node->is_leaf) {
        uint32_t end_left = linearizeBvh(node->left, flat, idx);
        uint32_t end_right = linearizeBvh(node->right, flat, idx);
        end_idx = end_right; // this subtree ends after right child
    } else {
        end_idx = my_idx + 1; // leaf subtree ends at next node
    }

    flat[my_idx].skip_index = (int32_t)end_idx;
    delete node;
    return end_idx;
}

// ============================================================================
// Write .ont binary
// ============================================================================

// ---------------------------------------------------------------------------
// 2.0c.1 — validacion previa a emitir .ont
//
// 2.0c.3b (DL-36): la validacion es un dry-run de sdfTreeToBytecode. Antes se
// muestreaba la expresion en tres puntos; eso no detectaba `%`, `tanh`,
// `sample` ni variables de sombreado, y ademas duplicaba la logica del emisor.
// Ahora validacion y emision son la misma funcion: si el emisor acepta la
// escena, la validacion pasa; si la rechaza, el error lleva primitiva,
// parametro, expresion y linea.
// ---------------------------------------------------------------------------
static void validateNodeSdfForOnt(const Node& n) {
    if (n.sdf_nodes.empty()) return;
    BytecodeBuilder scratch;
    sdfTreeToBytecode(n.sdf_nodes, 0, scratch);
}

void validateOntCompatibility(const Rih& rih) {
    for (const auto& n : rih.nodes) validateNodeSdfForOnt(n);
    for (const auto& n : rih.defs)  validateNodeSdfForOnt(n);
}

// 2.0c.2 (DL-34) / 2.0c.3b (DL-36): version = 2 si el bytecode usa opcodes
// nuevos (35..46) o si algun parametro SDF emitido es una expresion.
// El formato canonico es 5 bytes por instruccion, asi que se avanza de 5 en 5
// sin confundir el payload de ONT_CONST con un opcode.
static uint32_t ontVersionForBytecode(const std::vector<uint8_t>& bc,
                                      bool has_sdf_expr_params,
                                      uint64_t& feature_flags) {
    if (bc.size() % 5 != 0)
        throw std::runtime_error(
            "bytecode ONT invalido: " + std::to_string(bc.size()) +
            " bytes no es multiplo de 5");
    bool uses_new = false;
    for (size_t i = 0; i < bc.size(); i += 5) {
        uint8_t op = bc[i];
        if (!ontOpcodeIsKnown(op))
            throw std::runtime_error(
                "bytecode ONT usa un opcode desconocido: " +
                std::to_string((int)op));
        if (!ontOpcodeIsV1(op)) uses_new = true;
    }
    feature_flags = 0;
    if (uses_new) feature_flags |= ONT_FEATURE_MATH_EXTENDED;
    if (has_sdf_expr_params) feature_flags |= ONT_FEATURE_SDF_EXPR_PARAMS;
    return (feature_flags != 0) ? ONT_VERSION_V2 : ONT_VERSION_V1;
}

bool writeOnt(const Rih& rih, const std::string& path) {
    // --- Step 1: Flatten scene graph ---
    std::vector<OntFlatNode> flat_nodes;
    std::vector<bool> visited(rih.nodes.size() + rih.defs.size() + 1, false);

    for (auto& node : rih.nodes) {
        float root_xform[16]; mat4Identity(root_xform);
        flattenNode(node, rih, root_xform, flat_nodes, visited, 0.0f);
    }

    if (flat_nodes.empty()) {
        // Fallback: create a default sphere
        OntFlatNode fn;
        mat4Identity(fn.local_transform);
        fn.material_id = 0;
        fn.mode = 0;
        fn.id = 0;
        {
            BytecodeBuilder bb;
            bb.emit(ONT_VAR_X); bb.emit(ONT_VAR_X); bb.emit(ONT_MUL);
            bb.emit(ONT_VAR_Y); bb.emit(ONT_VAR_Y); bb.emit(ONT_MUL); bb.emit(ONT_ADD);
            bb.emit(ONT_VAR_Z); bb.emit(ONT_VAR_Z); bb.emit(ONT_MUL); bb.emit(ONT_ADD);
            bb.emit(ONT_SQRT); bb.emit(ONT_CONST, 1.0f); bb.emit(ONT_SUB);
            bb.emit(ONT_END);
            fn.bytecode = bb.bc;
        }
        fn.bbox_min[0] = fn.bbox_min[1] = fn.bbox_min[2] = fn.bbox_min[3] = -2;
        fn.bbox_max[0] = fn.bbox_max[1] = fn.bbox_max[2] = fn.bbox_max[3] = 2;
        fn.d_min = 0.0f;
        flat_nodes.push_back(fn);
    }

    // --- Step 2: Build BVH ---
    std::vector<OntFlatNode*> items(flat_nodes.size());
    for (size_t i = 0; i < flat_nodes.size(); i++) items[i] = &flat_nodes[i];

    uint32_t node_counter = 0;
    BvhBuildNode* root = buildBvhRecursive(items, 0, (uint32_t)items.size(), node_counter, 4);

    std::vector<BvhNode> bvh_flat;
    uint32_t bvh_idx = 0;
    linearizeBvh(root, bvh_flat, bvh_idx);

    // --- Step 3: Build bytecode block ---
    std::vector<uint8_t> bytecode_block;
    std::vector<uint32_t> bc_offsets(flat_nodes.size());
    for (size_t i = 0; i < flat_nodes.size(); i++) {
        bc_offsets[i] = (uint32_t)bytecode_block.size();
        bytecode_block.insert(bytecode_block.end(),
            flat_nodes[i].bytecode.begin(), flat_nodes[i].bytecode.end());
    }

    // --- Step 4: Prepare Materials ---
    std::vector<OntMaterial> ont_materials;
    if (rih.materials.empty()) {
        // Default gray material
        OntMaterial mat;
        mat.id = 0;
        mat.base_color[0] = 0.8f; mat.base_color[1] = 0.2f; mat.base_color[2] = 0.2f; mat.base_color[3] = 1.0f;
        mat.roughness = 0.3f;
        mat.metallic = 0.0f;
        mat.emission[0] = mat.emission[1] = mat.emission[2] = 0;
        mat.opacity = 1.0f;
        mat.tensor_slot = 0;
        ont_materials.push_back(mat);
    } else {
        for (auto& m : rih.materials) {
            OntMaterial mat;
            mat.id = m.id;
            mat.base_color[0] = m.tensor[4].is_expr ? 1 : m.tensor[4].constant;
            mat.base_color[1] = m.tensor[5].is_expr ? 1 : m.tensor[5].constant;
            mat.base_color[2] = m.tensor[6].is_expr ? 1 : m.tensor[6].constant;
            mat.base_color[3] = m.tensor[7].is_expr ? 1 : m.tensor[7].constant;
            mat.roughness = m.roughness;
            mat.metallic = m.metallic;
            mat.emission[0] = m.emission.r;
            mat.emission[1] = m.emission.g;
            mat.emission[2] = m.emission.b;
            mat.opacity = 1.0f;
            mat.tensor_slot = 0;
            ont_materials.push_back(mat);
        }
    }

    // --- Step 5: Compute scene AABB ---
    float scene_min[4] = { 1e9, 1e9, 1e9, 0 };
    float scene_max[4] = { -1e9, -1e9, -1e9, 0 };
    for (auto& fn : flat_nodes) {
        aabbUnion(scene_min, scene_max, fn.bbox_min, fn.bbox_max, scene_min, scene_max);
    }

    // --- Step 6: Write .ont file ---
    std::ofstream out(path, std::ios::binary);
    if (!out) return false;

    // Header
    OntHeader hdr = {};
    hdr.magic = ONT_MAGIC;
    // 2.0c.3b (DL-36): el flag se calcula sobre los nodos realmente emitidos.
    bool has_sdf_expr_params = false;
    for (auto& fn : flat_nodes) {
        if (fn.sdf_nodes && !fn.sdf_nodes->empty() &&
            sdfTreeHasExprParam(*fn.sdf_nodes, fn.sdf_root)) {
            has_sdf_expr_params = true;
            break;
        }
    }
    uint64_t feature_flags = 0;
    hdr.version = ontVersionForBytecode(bytecode_block, has_sdf_expr_params,
                                        feature_flags);
    hdr.reserved[0] = feature_flags;
    hdr.epsilon = rih.pipeline.trace_hit_threshold;
    hdr.node_count = (uint32_t)flat_nodes.size();
    hdr.bvh_count = (uint32_t)bvh_flat.size();
    hdr.material_count = (uint32_t)ont_materials.size();
    hdr.bytecode_size = (uint32_t)bytecode_block.size();
    hdr.tensor_buffer_size = (uint32_t)((flat_nodes.size() + 1) * 8 * sizeof(float)); // +1 = cámara slot 0
    for (int i = 0; i < 4; i++) {
        hdr.scene_aabb_min[i] = scene_min[i];
        hdr.scene_aabb_max[i] = scene_max[i];
    }
    out.write(reinterpret_cast<const char*>(&hdr), sizeof(hdr));

    // BvhNodes
    out.write(reinterpret_cast<const char*>(bvh_flat.data()),
              bvh_flat.size() * sizeof(BvhNode));

    // GraphNodes
    for (size_t i = 0; i < flat_nodes.size(); i++) {
        GraphNode gn;
        for (int j = 0; j < 16; j++)
            gn.local_transform[j] = flat_nodes[i].local_transform[j];
        gn.material_id = flat_nodes[i].material_id;
        gn.bytecode_offset = bc_offsets[i];
        gn.bytecode_length = (uint32_t)flat_nodes[i].bytecode.size();
        for (int j = 0; j < 4; j++) {
            gn.bbox_min[j] = flat_nodes[i].bbox_min[j];
            gn.bbox_max[j] = flat_nodes[i].bbox_max[j];
        }
        gn.tensor_slot = (uint32_t)i + 1; // slots 1..N (slot 0 = cámara)
        gn.mode = flat_nodes[i].mode;
        gn.pad[0] = gn.pad[1] = gn.pad[2] = 0;
        out.write(reinterpret_cast<const char*>(&gn), sizeof(gn));
    }

    // Bytecode block
    out.write(reinterpret_cast<const char*>(bytecode_block.data()), bytecode_block.size());

    // Materials
    out.write(reinterpret_cast<const char*>(ont_materials.data()),
              ont_materials.size() * sizeof(OntMaterial));

    return out.good();
}

} // namespace herm
