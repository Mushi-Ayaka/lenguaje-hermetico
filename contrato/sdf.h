#pragma once
#include "rih.h"
#include <functional>

// ============================================================================
// CONTRATO DE LA LIBRERÍA SDF
// Funciones de evaluación de campos de distancia.
// ============================================================================

namespace herm {
namespace sdf {

// ---------------------------------------------------------------------------
// Primitivas 3D
// ---------------------------------------------------------------------------

f32 sdSphere   (Vec3 p, f32 r);
f32 sdBox      (Vec3 p, Vec3 size);
f32 sdCylinder (Vec3 p, f32 r, f32 h);
f32 sdCapsule  (Vec3 p, Vec3 a, Vec3 b, f32 r);
f32 sdTorus    (Vec3 p, f32 r_major, f32 r_minor);
f32 sdPlane    (Vec3 p, Vec3 normal, f32 d);
f32 sdCone     (Vec3 p, f32 r, f32 h);
f32 sdRoundedBox(Vec3 p, Vec3 size, f32 r);
f32 sdOctahedron(Vec3 p, f32 r);

// ---------------------------------------------------------------------------
// Primitivas 4D
// ---------------------------------------------------------------------------

f32 sdSphere4D (Vec4 p, f32 r);
f32 sdTesseract(Vec4 p, f32 r);

// ---------------------------------------------------------------------------
// Operadores booleanos
// ---------------------------------------------------------------------------

f32 opUnion        (f32 a, f32 b);
f32 opSubtract     (f32 a, f32 b);
f32 opIntersect    (f32 a, f32 b);
f32 opSmoothUnion  (f32 a, f32 b, f32 k);
f32 opSmoothSubtract(f32 a, f32 b, f32 k);
f32 opSmoothIntersect(f32 a, f32 b, f32 k);

// ---------------------------------------------------------------------------
// Operadores de espacio
// ---------------------------------------------------------------------------

f32 opRepeat    (f32 d, f32 spacing);
Vec3 opRepeatDomain(Vec3 p, f32 spacing);
Vec3 opTwist    (Vec3 p, f32 angle);
Vec3 opElongate (Vec3 p, Vec3 size);

// ---------------------------------------------------------------------------
// Evaluación genérica desde SdfNode
// ---------------------------------------------------------------------------

// Callback para obtener distancia de un SdfNode por índice
using NodeEvalFn = std::function<f32(u32 node_id, Vec3 p)>;

// Evalúa un SdfNode en el punto p (coordenadas locales)
f32 evaluate(const SdfNode& node, Vec3 p, const NodeEvalFn& eval_child = {});

// Normal analítica (gradiente por diferencias finitas)
Vec3 normal(Vec3 p, const std::function<f32(Vec3)>& sdf_fn);

// ---------------------------------------------------------------------------
// Ray marching
// ---------------------------------------------------------------------------

struct Ray {
    Vec3 origin;
    Vec3 dir;
};

struct Hit {
    bool  hit;
    f32   t;           // distancia
    Vec3  point;       // punto de impacto
    Vec3  normal;      // normal en el impacto
    u32   material_id; // material del nodo impactado
};

Hit rayMarch(Ray ray, const std::function<f32(Vec3)>& sdf_fn, f32 t_min = 0.001f, f32 t_max = 100.0f, u32 max_steps = 256);

// ---------------------------------------------------------------------------
// Analytical displacement
// ---------------------------------------------------------------------------

// Evalúa una expresión analítica en el punto p y tiempo t
// Solo admite funciones deterministas: sin, cos, abs, sqrt, +, -, *, /, ^
f32 evalExpr(const std::string& expr, Vec3 p, f32 t);

} // namespace sdf
} // namespace herm
