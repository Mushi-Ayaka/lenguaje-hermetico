#pragma once
#include <cstdint>
#include <cstddef>
#include <cstdio>

// ============================================================================
// Formato .ont — Ontológico Binario
// Versión 1 (v2 comparte layout; ver feature_flags más abajo)
//
// Layout del archivo:
//   [OntHeader]
//   [BvhNode x bvh_count]
//   [GraphNode x node_count]
//   [bytecode: bytecode_size bytes]
//   [Material x material_count]
//
// Todo little-endian, sin padding.
// ============================================================================

#pragma pack(push, 1)

// Cabecera del archivo .ont
struct OntHeader {
    uint32_t magic      = 0x20544E4F;  // 'ONT '
    uint32_t version    = 1;
    float    epsilon    = 0.001f;       // umbral global de hit
    uint32_t node_count;                // número de GraphNodes
    uint32_t bvh_count;                 // número de BvhNodes
    uint32_t material_count;
    uint32_t bytecode_size;             // bytes totales del bloque bytecode
    uint32_t tensor_buffer_size;        // (N_nodos + 1) * 8 * 4 bytes (+1 = cámara slot 0)
    float    scene_aabb_min[4];         // AABB global de la escena (para ray start)
    float    scene_aabb_max[4];
    uint64_t reserved[7] = {};          // [0] = feature_flags (v2+); [1..6] = 0
};

// Nodo del BVH linealizado (para traversal con skip_index)
struct BvhNode {
    float    min[4];           // AABB 4D: xmin, ymin, zmin, wmin
    float    max[4];           // xmax, ymax, zmax, wmax
    float    d_min;            // distancia mínima garantizada a superficie
    float    L;                // constante de Lipschitz local
    int32_t  skip_index;       // índice al siguiente nodo (macro-salto)
    uint32_t first_node;       // índice al primer GraphNode en esta región
    uint16_t node_count;       // cuántos GraphNodes hay aquí
    uint16_t flags;            // bit 0: es_hoja
};

// Nodo del grafo ontológico (una instancia SDF)
struct GraphNode {
    float    local_transform[16]; // matriz 4x4 column-major (world → local)
    uint32_t material_id;
    uint32_t bytecode_offset;     // offset en bytes dentro del bloque bytecode
    uint32_t bytecode_length;     // longitud del bytecode de este nodo
    float    bbox_min[4];         // AABB local (x,y,z,w)
    float    bbox_max[4];
    uint32_t tensor_slot;         // índice en tensor_buffer (1..N) — por INSTANCIA
    uint8_t  mode;                // 0=solid, 1=volume
    uint8_t  pad[3];
};

// Entrada en tabla de materiales
struct OntMaterial {
    uint32_t id;
    float    base_color[4];  // RGBA
    float    roughness;
    float    metallic;
    float    emission[3];
    float    opacity;
    uint32_t tensor_slot;    // mismo slot que el nodo al que aplica
    uint32_t reserved[3];
};

#pragma pack(pop)

// ============================================================================
// Formato .obs — Observación (cámara, luces, timeline, metadatos)
// Archivo opcional que acompaña al .ont
//
// Layout:
//   [ObsHeader 64 bytes]
//   [ObsCamera 48 bytes]            (si flags & OBS_CAMERA)
//   [ObsLightsHeader + light_count * ObsLight] (si flags & OBS_LIGHTS)
//   [ObsTimeline]                    (si flags & OBS_TIMELINE)
//   [float bg[3]]                    (si flags & OBS_BACKGROUND)
//   [uint32_t width, height]         (si flags & OBS_RESOLUTION)
// ============================================================================

#pragma pack(push, 1)

struct ObsHeader {
    uint32_t magic     = 0x2053424F; // "OBS "
    uint32_t version   = 1;
    uint32_t flags;       // OBSFlags bitfield
    uint32_t reserved0;
    // Offsets desde inicio del archivo (0 = sección no presente)
    uint32_t camera_offset;
    uint32_t lights_offset;
    uint32_t timeline_offset;
    uint32_t background_offset;
    uint32_t resolution_offset;
    uint64_t reserved[4];
};

static constexpr uint32_t OBS_CAMERA     = 1u << 0;
static constexpr uint32_t OBS_LIGHTS     = 1u << 1;
static constexpr uint32_t OBS_TIMELINE   = 1u << 2;
static constexpr uint32_t OBS_BACKGROUND = 1u << 3;
static constexpr uint32_t OBS_RESOLUTION = 1u << 4;
static constexpr uint32_t OBS_MAGIC      = 0x2053424F;

struct ObsCamera {
    float position[3];
    float target[3];
    float up[3];
    float fov;
    float aperture;
    float focal_dist;
};

enum ObsLightType : uint8_t {
    OBS_LIGHT_DIRECTIONAL = 0,
    OBS_LIGHT_POINT       = 1
};

struct ObsLight {
    uint8_t type;     // ObsLightType
    uint8_t pad[3];
    float direction[3];
    float position[3];
    float color[3];
    float intensity;
    float falloff;
};

struct ObsLightsHeader {
    uint32_t count;
    // ObsLight[count] sigue inmediatamente
};

struct ObsTimeline {
    uint32_t w_frames;
    float w_min;
    float w_max;
    uint32_t t_unit_len; // 0 = usar "frame" por defecto
    // char t_unit[t_unit_len] (sin null terminator)
};

#pragma pack(pop)

// ============================================================================
// Opcodes del bytecode SDF (unificados con Herm OpCode)
// ============================================================================

enum OntOpcode : uint8_t {
    ONT_CONST    = 0,   // payload: siguiente float en el bytecode
    ONT_VAR_X,          // push variable x
    ONT_VAR_Y,          // push variable y
    ONT_VAR_Z,          // push variable z
    ONT_VAR_W,          // push variable w
    ONT_VAR_NX,         // push normal x
    ONT_VAR_NY,         // push normal y
    ONT_VAR_NZ,         // push normal z
    ONT_VAR_VX,         // push view x
    ONT_VAR_VY,         // push view y
    ONT_VAR_VZ,         // push view z
    ONT_VAR_LX,         // push light x
    ONT_VAR_LY,         // push light y
    ONT_VAR_LZ,         // push light z
    ONT_ADD,            // pop b, pop a, push a+b
    ONT_SUB,            // pop b, pop a, push a-b
    ONT_MUL,            // pop b, pop a, push a*b
    ONT_DIV,            // pop b, pop a, push a/b
    ONT_MOD,            // pop b, pop a, push fmod(a,b)
    ONT_NEG,            // pop, push -val
    ONT_POW,            // pop b, pop a, push pow(a,b)
    ONT_SIN,            // pop, push sin(val)
    ONT_COS,            // pop, push cos(val)
    ONT_TAN,            // pop, push tan(val)
    ONT_ABS,            // pop, push abs(val)
    ONT_SQRT,           // pop, push sqrt(val)
    ONT_FLOOR,          // pop, push floor(val)
    ONT_CEIL,           // pop, push ceil(val)
    ONT_MIN,            // pop b, pop a, push min(a,b)
    ONT_MAX,            // pop b, pop a, push max(a,b)
    ONT_CLAMP,          // pop hi, pop lo, pop v, push clamp(v,lo,hi)
    ONT_LERP,           // pop t, pop b, pop a, push lerp(a,b,t)
    ONT_MIX,            // igual que LERP (alias)
    ONT_SAMPLE,         // pop dz, dy, dx → recursive eval(x+dx,y+dy,z+dz)
    ONT_END,            // fin: resultado en tope de pila
};

// ============================================================================
// Versionado y feature_flags (2.0c.2, DL-34)
// ============================================================================

static constexpr uint32_t ONT_VERSION_V1 = 1;
static constexpr uint32_t ONT_VERSION_V2 = 2;
static constexpr uint32_t ONT_VERSION_MAX_SUPPORTED = ONT_VERSION_V2;

// reserved[0] de OntHeader. Solo tiene significado en v2+.
static constexpr uint64_t ONT_FEATURE_MATH_EXTENDED   = 1ull << 0;
static constexpr uint64_t ONT_FEATURE_SDF_EXPR_PARAMS = 1ull << 1;
static constexpr uint64_t ONT_FEATURE_KNOWN_MASK =
    ONT_FEATURE_MATH_EXTENDED | ONT_FEATURE_SDF_EXPR_PARAMS;

// Primer opcode nuevo. ONT_END = 35; los nuevos empiezan en 36.
static constexpr uint8_t ONT_FIRST_NEW_OPCODE = (uint8_t)(ONT_END + 1);

// ============================================================================
// Nombre de opcode para debug
// ============================================================================

inline const char* ontOpcodeName(OntOpcode op) {
    switch (op) {
        case ONT_CONST:    return "CONST";
        case ONT_VAR_X:    return "VAR_X";
        case ONT_VAR_Y:    return "VAR_Y";
        case ONT_VAR_Z:    return "VAR_Z";
        case ONT_VAR_W:    return "VAR_W";
        case ONT_VAR_NX:   return "VAR_NX";
        case ONT_VAR_NY:   return "VAR_NY";
        case ONT_VAR_NZ:   return "VAR_NZ";
        case ONT_VAR_VX:   return "VAR_VX";
        case ONT_VAR_VY:   return "VAR_VY";
        case ONT_VAR_VZ:   return "VAR_VZ";
        case ONT_VAR_LX:   return "VAR_LX";
        case ONT_VAR_LY:   return "VAR_LY";
        case ONT_VAR_LZ:   return "VAR_LZ";
        case ONT_ADD:      return "ADD";
        case ONT_SUB:      return "SUB";
        case ONT_MUL:      return "MUL";
        case ONT_DIV:      return "DIV";
        case ONT_MOD:      return "MOD";
        case ONT_NEG:      return "NEG";
        case ONT_POW:      return "POW";
        case ONT_SIN:      return "SIN";
        case ONT_COS:      return "COS";
        case ONT_TAN:      return "TAN";
        case ONT_ABS:      return "ABS";
        case ONT_SQRT:     return "SQRT";
        case ONT_FLOOR:    return "FLOOR";
        case ONT_CEIL:     return "CEIL";
        case ONT_MIN:      return "MIN";
        case ONT_MAX:      return "MAX";
        case ONT_CLAMP:    return "CLAMP";
        case ONT_LERP:     return "LERP";
        case ONT_MIX:      return "MIX";
        case ONT_SAMPLE:   return "SAMPLE";
        case ONT_END:      return "END";
        default: return "?";
    }
}

// Un opcode es v1 si cabe hasta ONT_END. Los nuevos viven detrás.
inline bool ontOpcodeIsV1(uint8_t op) {
    return op <= (uint8_t)ONT_END;
}

// ============================================================================
// Constantes helper
// ============================================================================

static constexpr uint32_t ONT_MAGIC     = 0x20544E4F;
static constexpr uint16_t BVH_FLAG_LEAF = 1;
static constexpr uint8_t  NODE_SOLID    = 0;
static constexpr uint8_t  NODE_VOLUME   = 1;


