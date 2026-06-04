#pragma once
#include "rih.h"
#include "sdf.h"
#include <vector>
#include <functional>

// ============================================================================
// CONTRATO DE LA LIBRERÍA DE JERARQUÍA (Scene Graph)
// Nodos, transformaciones, herencia y BVH.
// ============================================================================

namespace herm {
namespace scene {

// ---------------------------------------------------------------------------
// Árbol de escena
// ---------------------------------------------------------------------------

struct SceneGraph {
    Scene           scene;
    std::vector<Material> materials;
    std::vector<Node>     nodes;     // todos los nodos (groups, sdf, instances)
    std::vector<Node>     defs;      // definiciones reutilizables
};

// Construye el grafo desde una RIH
SceneGraph fromRih(const Rih& rih);

// ---------------------------------------------------------------------------
// Resolución de herencia
// ---------------------------------------------------------------------------

// Calcula la transformación mundo de un nodo (multiplicando padres)
Mat4 worldTransform(const SceneGraph& graph, u32 node_id);

// Resuelve el material efectivo de un nodo (hereda del padre si no tiene)
u32 resolveMaterial(const SceneGraph& graph, u32 node_id);

// ---------------------------------------------------------------------------
// Evaluación del grafo para ray marching
// ---------------------------------------------------------------------------

// Evalúa la SDF combinada de todo el grafo en el punto p (mundo)
// Resuelve herencia, transforms, instances, y operadores booleanos
struct EvalResult {
    f32  distance;
    u32  material_id;
    bool is_volume;
    f32  density_scale;
};

EvalResult evaluate(const SceneGraph& graph, Vec3 p, f32 t = 0.0f);

// ---------------------------------------------------------------------------
// BVH (Bounding Volume Hierarchy)
// ---------------------------------------------------------------------------

struct BvhNode {
    Vec3 bbox_min;
    Vec3 bbox_max;
    u32  node_id;       // índice al Node del SceneGraph (0xFFFFFFFF = interno)
    u32  child_left;    // índice al BvhNode hijo
    u32  child_right;
    bool is_leaf;
};

struct Bvh {
    std::vector<BvhNode> nodes;

    // Construye BVH desde el grafo
    static Bvh build(const SceneGraph& graph);

    // Consulta: devuelve lista de node_ids que intersectan el rayo
    std::vector<u32> query(const Ray& ray) const;
};

// ---------------------------------------------------------------------------
// Instanciación
// ---------------------------------------------------------------------------

// Expande una instancia: copia el nodo def con la transform del instance
Node expandInstance(const Node& instance_node, const Node& def_node);

// ---------------------------------------------------------------------------
// Animación
// ---------------------------------------------------------------------------

// Evalúa todas las expresiones animadas en el grafo para el tiempo t
void evaluateExpressions(SceneGraph& graph, f32 t);

// ---------------------------------------------------------------------------
// Serialización
// ---------------------------------------------------------------------------

Rih toRih(const SceneGraph& graph);
SceneGraph fromRih(const Rih& rih);

} // namespace scene
} // namespace herm
