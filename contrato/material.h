#pragma once
#include "rih.h"
#include <string>
#include <vector>

// ============================================================================
// CONTRATO DE LA LIBRERÍA DE MATERIALES
// Catálogo de materiales predefinidos y utilidades.
// ============================================================================

namespace herm {
namespace material {

// ---------------------------------------------------------------------------
// Catálogo de materiales predefinidos
// ---------------------------------------------------------------------------

struct CatalogEntry {
    std::string name;
    Material    material;
};

// Devuelve el catálogo completo de materiales predefinidos
const std::vector<CatalogEntry>& catalog();

// Busca un material por nombre en el catálogo
std::optional<Material> find(const std::string& name);

// ---------------------------------------------------------------------------
// Materiales predefinidos (selección)
// ---------------------------------------------------------------------------

// Metales
Material metal(const ColorRGB& base, f32 roughness = 0.2f);
Material gold(f32 roughness = 0.1f);
Material copper(f32 roughness = 0.2f);
Material iron(f32 roughness = 0.3f);
Material silver(f32 roughness = 0.1f);

// Dieléctricos
Material glass(ColorRGB tint = {0.9f, 0.95f, 1.0f}, f32 ior = 1.5f);
Material water(f32 ior = 1.33f);
Material diamond(f32 ior = 2.42f);

// Orgánicos
Material wood(ColorRGB base = {0.55f, 0.27f, 0.07f}, f32 roughness = 0.8f);
Material leather(ColorRGB base = {0.3f, 0.15f, 0.05f}, f32 roughness = 0.9f);
Material stone(ColorRGB base = {0.5f, 0.5f, 0.5f}, f32 roughness = 0.95f);

// Emisivos
Material light(ColorRGB emission, f32 intensity = 1.0f);
Material sun(f32 intensity = 10.0f);

// ---------------------------------------------------------------------------
// Utilidades
// ---------------------------------------------------------------------------

// Valida que un material tenga valores en rango
bool validate(const Material& mat, std::string* error_msg = nullptr);

// Mezcla dos materiales (para nodos sin material definido que heredan)
Material blend(const Material& parent, const Material& child, f32 child_weight = 1.0f);

// Serializa un material a string (formato .herm)
std::string toString(const Material& mat);

} // namespace material
} // namespace herm
