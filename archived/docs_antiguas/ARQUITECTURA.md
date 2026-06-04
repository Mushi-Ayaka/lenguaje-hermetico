# Arquitectura del Lenguaje Hermético

## 1. Principios

- **Fuera de MLIR**: el lenguaje es independiente, no está dentro del pipeline MLIR
- **Salida simple**: produce una Representación Intermedia Hermética (RIH), no se adapta a MLIR
- **TB como backend principal**: RIH se traduce al formato `.tb` del MLIR (bloques 8×8 comprimidos, el mismo que produce `ctb.exe`)
- **Render**: lo hace el `render.exe` existente (`.tb` → `.png`)
- **MLIR como opción**: el driver MLIR (RIH → `.gte`) es una alternativa cuando convenga
- **Determinismo**: toda función en la RIH debe ser analítica; no se admite ruido estadístico no acotado

## 2. Diagrama de Componentes

```
┌──────────────────────────────────────────────────────────┐
│              H ERM (herramienta unificada)                │
│  Lee .herm → Resuelve matemáticas → Emite RIH            │
│  Unifica compilador + resolvedor en un solo binario      │
│                                                          │
│  ┌─────────────┐  ┌──────────────┐  ┌────────────────┐  │
│  │ Parser .herm │  │ Resolvedor   │  │ Generador RIH  │  │
│  │ (AST → grafo │  │ (evalúa SDF, │  │ (JSON y/o      │  │
│  │  de escena)  │  │  herencia,   │  │  binario)      │  │
│  │              │  │  animación)  │  │                │  │
│  └──────┬───────┘  └──────┬───────┘  └───────┬────────┘  │
│         │                 │                  │           │
│         └─────────────────┴──────────────────┘           │
└─────────────────────────────┬────────────────────────────┘
                              │
                              ▼ RIH
                    ┌─────────────────┐
                    │  RIH  (.rih)    │
                    │ JSON o binario  │
                    └───┬──────┬──────┘
                        │      │
          ┌─────────────┘      └─────────────┐
          ▼                                  ▼
┌──────────────────┐               ┌──────────────────┐
│  OPERADOR TB     │               │  DRIVER MLIR     │
│  RIH → .tb       │               │  RIH → .gte      │
│  (sphere tracing │               │  (opcional)      │
│   o bisección)   │               │                  │
└──────┬───────────┘               └──────┬───────────┘
       │                                  │
       ▼                                  ▼
┌──────────────┐                  ┌──────────────┐
│  render.exe  │                  │  MLIR pipe    │
│  .tb → .png  │                  │  .gte→.png    │
└──────────────┘                  └──────────────┘
```

## 3. Flujo de Compilación

### 3.1 Modo TB (principal)
```
.herm → herm → RIH → operador_tb → .tb → render.exe → .png
```

### 3.2 Modo MLIR (opcional)
```
.herm → herm → RIH → driver_mlir → .gte → gte.exe → .ptr → ptr.exe → .ctp → ctb.exe → .tb → render.exe → .png
```

## 4. La Herramienta `herm` (Compilador + Resolvedor Unificados)

Binario único que realiza todo el pipeline del Lenguaje Hermético:

| Fase | Descripción |
|------|-------------|
| **Parse** | Lee `.herm`, construye AST, genera grafo de escena con nodos, materiales, SDFs |
| **Resolver** | Evalúa SDFs, aplica operadores (Perturbación, Influencia, Aniquilación), resuelve herencia de transformaciones y materiales, evalúa expresiones animadas (con `t`) |
| **Emit** | Genera RIH en JSON o binario |

**Flags de línea de comandos:**
```
herm entrada.herm -o salida.rih          # RIH en JSON (default)
herm entrada.herm -o salida.rih --binary # RIH en binario
herm entrada.herm -o salida.rih --tb     # RIH → .tb directo
herm entrada.herm -o salida.gte          # RIH → .gte (driver MLIR)
herm entrada.herm --render               # todo hasta .png
```

## 5. Representación Intermedia Hermética (RIH)

### 5.1 Formato JSON

```json
{
  "version": 1,
  "scene": {
    "axes": { "X": 1920, "Y": 1080, "W": 60 },
    "time": {
      "w_min": 0,
      "w_max": 59,
      "t_unit": "frame"
    },
    "camera": {
      "position": [0, 2, 5],
      "target": [0, 0, 0],
      "up": [0, 1, 0],
      "fov": 60,
      "aperture": 0.0,
      "focal_dist": 10.0
    },
    "background": [0.05, 0.05, 0.1]
  },
  "materials": [
    {
      "id": 0,
      "name": "madera",
      "base_color": [0.55, 0.27, 0.07],
      "roughness": 0.8,
      "metallic": 0.0,
      "emission": [0, 0, 0],
      "emission_mode": "isotropic",
      "ior": 1.5,
      "transmission": 0.0,
      "opacity": 1.0,
      "subsurface": 0.0,
      "blend_mode": "alpha"
    },
    {
      "id": 1,
      "name": "metal",
      "base_color": [0.8, 0.8, 0.8],
      "roughness": 0.2,
      "metallic": 1.0,
      "emission": [0, 0, 0],
      "emission_mode": "isotropic",
      "ior": 1.0,
      "transmission": 0.0,
      "opacity": 1.0,
      "subsurface": 0.0,
      "blend_mode": "replace"
    }
  ],
  "defs": [
    {
      "id": 0,
      "name": "pata",
      "node": {
        "mode": "solid",
        "sdf": { "type": "cylinder", "params": { "radius": 0.05, "height": 0.8 } },
        "material": 1
      }
    }
  ],
  "nodes": [
    {
      "id": 0,
      "name": "silla",
      "type": "group",
      "transform": {
        "translate": [0, 0, 0],
        "rotate": [0, 0, 0],
        "scale": [1, 1, 1]
      },
      "children": [1, 2, 3]
    },
    {
      "id": 1,
      "name": "asiento",
      "type": "sdf",
      "mode": "solid",
      "sdf": { "type": "box", "params": { "size": [1.0, 0.1, 0.8] } },
      "material": 0,
      "transform": {
        "translate": [0, 0.4, 0],
        "rotate": [0, 0, 0],
        "scale": [1, 1, 1]
      },
      "density_scale": 1.0,
      "blend_mode": "alpha"
    },
    {
      "id": 2,
      "name": "respaldo",
      "type": "sdf",
      "mode": "solid",
      "sdf": { "type": "box", "params": { "size": [0.8, 0.6, 0.08] } },
      "material": 0,
      "transform": {
        "translate": [0, 0.9, -0.35],
        "rotate": [5, 0, 0],
        "scale": [1, 1, 1]
      }
    },
    {
      "id": 3,
      "name": "pata_1",
      "type": "instance",
      "def": 0,
      "transform": {
        "translate": [-0.4, 0.2, -0.3],
        "rotate": [0, 0, 0],
        "scale": [1, 1, 1]
      }
    }
  ]
}
```

### 5.2 Animación en RIH (expresiones con `t`)

Las transformaciones animadas se almacenan como expresiones en lugar de valores fijos:

```json
{
  "id": 4,
  "name": "estrella",
  "type": "sdf",
  "mode": "solid",
  "sdf": { "type": "sphere", "params": { "radius": 0.3 } },
  "material": 1,
  "transform": {
    "translate": [
      { "expr": "sin(t * 2) * 3" },
      { "expr": "cos(t * 1.5) * 2" },
      0
    ],
    "rotate": [0, { "expr": "t * 30" }, 0],
    "scale": [1, 1, 1]
  }
}
```

Cuando un valor es un número fijo, se usa como escalar. Cuando es un objeto `{"expr": "..."}`, es una expresión analítica que depende de `t`.

### 5.3 SDF con displacement analítico (sin ruido)

```json
{
  "id": 5,
  "name": "montaña",
  "type": "sdf",
  "mode": "solid",
  "sdf": {
    "type": "displace",
    "a": { "type": "plane", "params": { "normal": [0, 1, 0], "d": 0 } },
    "f": "sin(x * 5) * cos(z * 5) * 0.1 + sin(x * 13 + z * 7) * 0.05"
  },
  "material": 0
}
```

No se admite `noise()` ni `fbm()` como funciones built-in. Solo expresiones analíticas deterministas.

### 5.4 Modo volumen

```json
{
  "id": 6,
  "name": "niebla",
  "type": "sdf",
  "mode": "volume",
  "sdf": { "type": "sphere", "params": { "radius": 10.0 } },
  "material": 2,
  "density_scale": 0.5
}
```

### 5.5 Formato Binario (.rib)

Además de JSON, se define un formato binario compacto para carga rápida en el motor de juego:

```
RIH Binario (.rib)
──────────────────

[Header - 32 bytes]
  magic:       u32   = 0x52494848        ("RIHH" little-endian)
  version:     u32   = 1
  scene_flags: u32   (bit 0: has_camera, bit 1: has_time, ...)
  num_materials: u32
  num_nodes:     u32
  num_defs:      u32
  reserved:    u64   = 0

[Material Table - num_materials × 128 bytes cada uno]
  id:           u32
  name_offset:  u32   (offset en string table)
  base_color:   f32[3]
  roughness:    f32
  metallic:     f32
  emission:     f32[3]
  emission_mode: u8   (0=isotropic, 1=directional)
  emission_dir:  f32[3]
  ior:          f32
  transmission: f32
  opacity:      f32
  subsurface:   f32
  blend_mode:   u8   (0=alpha, 1=add, 2=replace)
  padding:      u8[14]

[Node Table - entrada variable, cada nodo tiene:]
  id:           u32
  type:         u8   (0=group, 1=sdf, 2=instance)
  mode:         u8   (0=solid, 1=volume)
  name_offset:  u32
  material_id:  u32   (0xFFFFFFFF = none)
  density_scale: f32
  blend_mode:   u8
  padding:      u8[7]
  
  [Transform - 72 bytes]
    translate: f32[3] o string_offset (si es expresión)
    rotate:    f32[3] o string_offset
    scale:     f32[3] o string_offset
  
  [SDF node adicional:]
    sdf_type:   u8   (0=sphere, 1=box, 2=cylinder, ...)
    params:     f32[4] (máximo 4 params)
    operator children: u32[2] (para union/subtract/etc)
    displace_expr_offset: u32 (0 = none)
  
  [Group node adicional:]
    num_children: u32
    children:     u32[num_children]
  
  [Instance node adicional:]
    def_id: u32

[String Table]
  offsets_concatendadas + datos utf-8

[Scene]
  axes:         u32[3]  (X, Y, W)
  time_w_min:   f32
  time_w_max:   f32
  t_unit:       u8 (0=frame, 1=second)
  camera:       (solo si scene_flags & 1)
    position:   f32[3]
    target:     f32[3]
    up:         f32[3]
    fov:        f32
    aperture:   f32
    focal_dist: f32
  background:   f32[3]
```

## 6. Operador TB

Traduce RIH → `.tb` (el formato del MLIR: bloques 8×8 comprimidos, el mismo que usa `ctb.exe` y `render.exe`).

### Algoritmo de renderizado

Para cada fotograma `t` en `[W_min, W_max]`:

1. **Inicializar**: fijar `W = g(t)` (mapeo lineal o el definido en escena)
2. **Por cada píxel** `(x, y)`:
   - Calcular rayo desde cámara en `(x, y)`
   - Recorrer el grafo de escena (nodos SDF con transforms) usando **sphere tracing** (offline) o **bisección afín** (si se busca consistencia con el motor de juego)
   - Evaluar material en punto de impacto (color, roughness, etc.)
   - Si `mode = volume`, integrar densidad a lo largo del rayo
3. **Empaquetar** el buffer RGBA resultante en formato `.tb` (bloques 8×8 comprimidos)

### Sphere tracing vs bisección afín

| Método | Uso |
|--------|-----|
| **Sphere tracing** | Renderizado offline (más rápido cuando no hay garantía Lipschitz global) |
| **Bisección afín** | Cuando se necesita coherencia byte-exacta con el motor de juego |

## 7. Librerías

### 7.1 Librería de Materiales
- `libmaterial/` — tipos y propiedades de materiales
- Cada material: `base_color`, `roughness`, `metallic`, `emission`, `emission_mode`, `ior`, `transmission`, `opacity`, `subsurface`, `blend_mode`
- Paletas predefinidas, catálogo expandible
- Independiente del lenguaje (se puede usar sola)

### 7.2 Librería de Jerarquía (Scene Graph)
- `libjerarquia/` — nodos, transformaciones, herencia
- SDFs primitivas: `box`, `sphere`, `cylinder`, `torus`, `plane`, `capsule`, `cone`, `rounded_box`, `octahedron`, `tesseract`, `sphere4d`
- Operadores SDF: `union`, `subtract`, `intersect`, `smooth_union`, `smooth_subtract`, `smooth_intersect`, `repeat`, `twist`, `elongate`, `displace`
- Transformaciones anidadas con herencia matriz-padre
- BVH para aceleración espacial
- Independiente del lenguaje

### 7.3 Librería de SDFs (base matemática)
- `libsdf/` — implementación de campos de distancia escalares
- Funciones: `sdBox`, `sdSphere`, `sdCylinder`, `sdTorus`, etc.
- Operadores: `opUnion`, `opSubtract`, `opIntersect`, `opSmoothUnion`

## 8. Driver MLIR (Opcional)

Traduce RIH → `.gte`:

1. Recibe la RIH
2. Genera código `.gte` equivalente (expresiones por píxel con ray marching)
3. El pipeline MLIR existente lo procesa: `.gte` → `.ptr` → `.ctp` → `.tb` → `.png`

Solo si es más fácil o necesario. Alternativa: el operador TB puede emitir `.tb` directamente.

## 9. Estructura de Directorios

```
Lenguaje Hermético/
├── ARQUITECTURA.md              ← este documento
├── SINTAXIS.md                  ← sintaxis del lenguaje .herm
├── docs/                        # documentación adicional
├── libmaterial/                 # librería de materiales
├── libjerarquia/                # librería de jerarquía (scene graph)
├── libsdf/                      # librería de SDFs matemáticas
├── herm/                        # herramienta unificada (.herm → RIH)
├── operador_tb/                 # RIH → .tb
├── driver_mlir/                 # RIH → .gte (opcional)
└── ejemplos/                    # archivos .herm de ejemplo
```
