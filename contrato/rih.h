#pragma once
#include <string>
#include <vector>
#include <variant>
#include <optional>
#include <cstdint>
#include <unordered_map>

// ============================================================================
// CONTRATO DEL LENGUAJE HERMÉTICO
// Definiciones de tipos compartidos entre herm, operador_tb, driver_mlir,
// librerías y motor de juego.
// ============================================================================

namespace herm {

// ---------------------------------------------------------------------------
// Tipos base
// ---------------------------------------------------------------------------

using f32 = float;
using u32 = uint32_t;
using u8  = uint8_t;
using u64 = uint64_t;

struct Vec2 { f32 x, y; };
struct Vec3 { f32 x, y, z; };
struct Vec4 { f32 x, y, z, w; };
struct Mat4 { f32 m[16]; }; // column-major

struct ColorRGB { f32 r, g, b; };

// ---------------------------------------------------------------------------
// Expresión analítica (puede ser un valor fijo o una expresión con t)
// ---------------------------------------------------------------------------

struct Expr {
    bool is_expr;         // false = valor fijo, true = expresión
    f32  constant;        // valor cuando is_expr = false
    std::string expression; // "sin(t*2)*3" cuando is_expr = true
};

// ---------------------------------------------------------------------------
// Blend mode
// ---------------------------------------------------------------------------

enum class BlendMode : u8 {
    ALPHA   = 0,
    ADD     = 1,
    REPLACE = 2
};

// ---------------------------------------------------------------------------
// Emission mode
// ---------------------------------------------------------------------------

enum class EmissionMode : u8 {
    ISOTROPIC   = 0,
    DIRECTIONAL = 1
};

// ============================================================================
// LIGHT
// ============================================================================

enum class LightType : u8 {
    DIRECTIONAL = 0,
    POINT       = 1
};

struct Light {
    u32          id;
    std::string  name;
    LightType    type         = LightType::DIRECTIONAL;
    Vec3         direction    = {0.3f, 0.8f, 0.5f};
    Vec3         position     = {0.0f, 0.0f, 0.0f};
    ColorRGB     color        = {1.0f, 1.0f, 1.0f};
    f32          intensity    = 1.0f;
    f32          falloff      = 0.0f; // for point lights
};

// ============================================================================
// MATERIAL
// ============================================================================

struct Material {
    u32          id;
    std::string  name;
    Expr         tensor[8]; // [x, y, z, w, r, g, b, a]
    BlendMode    blend_mode    = BlendMode::ALPHA;
};

// ============================================================================
// SDF (Signed Distance Function)
// ============================================================================

enum class SdfType : u8 {
    SPHERE      = 0,
    BOX         = 1,
    CYLINDER    = 2,
    CAPSULE     = 3,
    TORUS       = 4,
    PLANE       = 5,
    CONE        = 6,
    ROUNDED_BOX = 7,
    OCTAHEDRON  = 8,
    TESSERACT   = 9,  // 4D hypercube
    SPHERE4D    = 10, // 4D hypersphere

    // Operators
    UNION            = 64,
    SUBTRACT         = 65,
    INTERSECT        = 66,
    SMOOTH_UNION     = 67,
    SMOOTH_SUBTRACT  = 68,
    SMOOTH_INTERSECT = 69,
    REPEAT           = 70,
    TWIST            = 71,
    ELONGATE         = 72,
    DISPLACE         = 73
};

struct SdfNode {
    SdfType type;

    // Parámetros de primitiva (hasta 4, c/u puede ser expresión o constante)
    Expr params[4] = {};

    // Para operadores binarios (union, subtract, ...)
    std::optional<u32> child_a; // índice a otro SdfNode
    std::optional<u32> child_b;

    // Para operadores con parámetro k (smooth_*)
    std::optional<f32> k;

    // Para REPEAT
    std::optional<f32> spacing;

    // Para DISPLACE: función analítica
    std::optional<std::string> displace_func; // ej. "sin(x*5)*cos(z*5)*0.1"
};

// ============================================================================
// TRANSFORMACIÓN
// ============================================================================

struct Transform {
    Expr translate[3]; // [x, y, z]
    Expr rotate[3];    // [x, y, z] en grados
    Expr scale[3];     // [x, y, z]
};

// ============================================================================
// MODO (sólido / volumen)
// ============================================================================

enum class NodeMode : u8 {
    SOLID  = 0,
    VOLUME = 1
};

// ============================================================================
// NODO DEL GRAFO DE ESCENA
// ============================================================================

enum class NodeType : u8 {
    GROUP    = 0,
    SDF      = 1,
    INSTANCE = 2
};

struct Node {
    u32       id;
    NodeType  type;
    std::string name;

    // Comunes
    NodeMode  mode          = NodeMode::SOLID;
    Transform transform;
    f32       density_scale = 1.0f;  // solo si mode = VOLUME
    BlendMode blend_mode    = BlendMode::ALPHA;

    // SDF node — flat list, index 0 = root (for SDF nodes)
    std::vector<SdfNode> sdf_nodes;
    std::optional<u32>   material_id; // índice en material table

    // Group node
    std::vector<u32> children;

    // Instance node
    std::optional<u32> def_id;  // índice en def table
};

// ============================================================================
// CÁMARA
// ============================================================================

struct Camera {
    Vec3 position  = {0.0f, 0.0f, 0.0f};
    Vec3 target    = {0.0f, 0.0f, 0.0f};
    Vec3 up        = {0.0f, 1.0f, 0.0f};
    f32  fov       = 60.0f;   // grados
    f32  aperture  = 0.0f;    // 0 = pinhole
    f32  focal_dist = 10.0f;
};

// ============================================================================
// ESCENA
// ============================================================================

struct Scene {
    u32  width  = 1920;
    u32  height = 1080;
    u32  w_frames = 1;      // número de cortes temporales (W)
    f32  w_min  = 0.0f;
    f32  w_max  = 0.0f;     // w_max = w_frames - 1 por defecto
    std::string t_unit = "frame";

    Camera       camera;
    ColorRGB     background = {0.0f, 0.0f, 0.0f};
};

// ============================================================================
// REPRESENTACIÓN INTERMEDIA HERMÉTICA (RIH)
// ============================================================================

struct Rih {
    u32 version = 1;
    Scene scene;

    std::vector<Light>    lights;
    std::vector<Material> materials;
    std::vector<Node>     defs;    // definiciones reutilizables (def)
    std::vector<Node>     nodes;   // grafo de escena
};

// ============================================================================
// FORMATO BINARIO (.rib) CONSTANTES
// ============================================================================

namespace binary {

constexpr u32 MAGIC = 0x52494848; // "RIHH"

#pragma pack(push, 1)
struct Header {
    u32 magic;
    u32 version;
    u32 scene_flags;  // bit 0: has_camera, bit 1: has_time
    u32 num_materials;
    u32 num_nodes;
    u32 num_defs;
    u32 num_lights;
    u32 reserved;       // padding to 32 bytes
};

struct LightRecord {
    u32  id;
    u32  name_offset;
    u8   light_type;    // 0=directional, 1=point
    f32  direction[3];
    f32  position[3];
    f32  color[3];
    f32  intensity;
    f32  falloff;
    u8   padding[4];
};

struct MaterialRecord {
    u32  id;
    u32  name_offset;
    union { f32 constant; u32 string_offset; } tensor[8];
    u8   tensor_is_expr; // bitmask (8 bits = 8 components)
    u8   blend_mode;
    u8   padding[2];
};

struct NodeRecord {
    u32  id;
    u8   type;       // NodeType
    u8   mode;       // NodeMode
    u32  name_offset;
    u32  material_id;
    f32  density_scale;
    u8   blend_mode;
    u8   padding[7];
    // Sigue: transform (72 bytes), luego datos según type
};

struct TransformRecord {
    union { f32 constant; u32 string_offset; } translate[3];
    union { f32 constant; u32 string_offset; } rotate[3];
    union { f32 constant; u32 string_offset; } scale[3];
};

struct SdfRecord {
    u8   sdf_type;
    union { f32 constant; u32 string_offset; } params[4];
    u32  child_a;
    u32  child_b;
    f32  k;
    f32  spacing;
    u32  displace_expr_offset; // 0 = none
};
#pragma pack(pop)

} // namespace binary

} // namespace herm
