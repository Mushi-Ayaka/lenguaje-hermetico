# Especificaciones Técnicas

## 1. Formato RIH JSON

### 1.1 Esquema

```json
{
  "version": 1,
  "scene": {
    "axes": { "X": 800, "Y": 600, "W": 1 },
    "time": { "w_min": 0, "w_max": 0, "t_unit": "frame" },
    "camera": {
      "position": [0, 0, 0],
      "target": [0, 0, 0],
      "up": [0, 1, 0],
      "fov": 60,
      "aperture": 0.0,
      "focal_dist": 10.0
    },
    "background": [0, 0, 0]
  },
  "materials": [ ... ],
  "defs": [ ... ],
  "nodes": [ ... ]
}
```

### 1.2 Material

```json
{
  "id": 0,
  "name": "madera",
  "base_color": [0.55, 0.27, 0.07],
  "roughness": 0.8,
  "metallic": 0.0,
  "emission": [0, 0, 0],
  "emission_mode": "isotropic",
  "emission_dir": [0, -1, 0],
  "ior": 1.5,
  "transmission": 0.0,
  "opacity": 1.0,
  "subsurface": 0.0,
  "blend_mode": "alpha"
}
```

### 1.3 Nodo

**SDF:**
```json
{
  "id": 1, "name": "esfera", "type": "sdf", "mode": "solid",
  "sdf": { "type": "sphere", "params": { "radius": 1.0 } },
  "material": 0,
  "transform": { "translate": [0, 0, 0], "rotate": [0, 0, 0], "scale": [1, 1, 1] }
}
```

**Group:**
```json
{
  "id": 0, "name": "silla", "type": "group",
  "transform": { ... },
  "children": [1, 2, 3]
}
```

**Instance:**
```json
{
  "id": 3, "name": "pata_1", "type": "instance",
  "def": 0,
  "transform": { ... }
}
```

### 1.4 Expresiones Animadas

```json
{
  "translate": [
    { "expr": "sin(t * 2) * 3" },
    { "expr": "cos(t * 1.5) * 2" },
    0
  ]
}
```

Las expresiones también se admiten en **parámetros de primitivas SDF** (`params`):

```json
{
  "type": "sphere",
  "params": [
    { "expr": "0.65 + sin(w * 2) * 0.35" },
    0.0,
    0.0,
    0.0
  ]
}
```

Cada elemento del array `params` puede ser:
- Un número (`f32`): valor constante.
- Un objeto `{ "expr": "..." }`: expresión analítica evaluada por fotograma.

Esto permite animar la geometría misma (radio pulsante, caja que crece, etc.) y usar parámetros de `def` en combinación con operaciones aritméticas.

### 1.5 Tipos SDF

| type | params | Descripción |
|------|--------|-------------|
| `sphere` | `radius: f64` | Esfera |
| `box` | `size: [f64, f64, f64]` | Caja |
| `cylinder` | `radius: f64, height: f64` | Cilindro en Y |
| `capsule` | `a: [f64, f64, f64], b: [f64, f64, f64], radius: f64` | Cápsula |
| `torus` | `radius_major: f64, radius_minor: f64` | Toro |
| `plane` | `normal: [f64, f64, f64], d: f64` | Plano infinito |
| `cone` | `radius: f64, height: f64` | Cono en Y |
| `rounded_box` | `size: [f64, f64, f64], radius: f64` | Caja con bordes redondeados |
| `octahedron` | `radius: f64` | Octaedro |
| `tesseract` | `radius: f64` | Hipercubo 4D |
| `sphere4d` | `radius: f64` | Hiperesfera 4D |

### 1.6 Operadores SDF

| type | hijos | params | Descripción |
|------|-------|--------|-------------|
| `union` | `a, b` | — | Unión (min) |
| `subtract` | `a, b` | — | Resta (max(a, -b)) |
| `intersect` | `a, b` | — | Intersección (max) |
| `smooth_union` | `a, b` | `k: f64` | Unión suave |
| `smooth_subtract` | `a, b` | `k: f64` | Resta suave |
| `smooth_intersect` | `a, b` | `k: f64` | Intersección suave |
| `repeat` | `a` | `spacing: [f64, f64, f64]` | Repetición espacial |
| `twist` | `a` | `angle: f64` | Torsión en Y |
| `elongate` | `a` | `size: [f64, f64, f64]` | Estiramiento |
| `displace` | `a` | `f: string` | Desplazamiento por expresión |

## 2. Formato RIH Binario (.rib)

### 2.1 Header (32 bytes)

```
magic:       u32  = 0x52494848  ("RIHH")
version:     u32  = 1
scene_flags: u32  (bit 0: has_camera, bit 1: has_time)
num_materials: u32
num_nodes:     u32
num_defs:      u32
reserved:    u64  = 0
```

### 2.2 Material Table (num_materials × 128 bytes)

```
id:           u32
name_offset:  u32
base_color:   f32[3]
roughness:    f32
metallic:     f32
emission:     f32[3]
emission_mode: u8  (0=isotropic, 1=directional)
emission_dir:  f32[3]
ior:          f32
transmission: f32
opacity:      f32
subsurface:   f32
blend_mode:   u8  (0=alpha, 1=add, 2=replace)
padding:      u8[14]
```

### 2.3 Nodos (tamaño variable)

**Header común (32 bytes):**
```
id:           u32
type:         u8  (0=group, 1=sdf, 2=instance)
mode:         u8  (0=solid, 1=volume)
name_offset:  u32
material_id:  u32  (0xFFFFFFFF = none)
density_scale: f32
blend_mode:   u8
padding:      u8[7]
```

**Transform (72 bytes):**
```
translate: f32[3] o string_offset (si expresión)
rotate:    f32[3] o string_offset
scale:     f32[3] o string_offset
```

**SDF adicional:**
```
sdf_type:   u8
params:     f32[4]
children:   u32[2]
displace_expr_offset: u32
```

**Group adicional:**
```
num_children: u32
children:     u32[num_children]
```

**Instance adicional:**
```
def_id: u32
```

## 3. Formato .tb (Render)

Formato de bloques 8×8 comprimidos, compatible con `render.exe`:

| Tipo de bloque | ID | Tamaño | Descripción |
|----------------|-----|--------|-------------|
| P0 | `0x00` | 4 bytes | Color sólido: RGBA (4 bytes) |
| PAL_RGBA | `0x04` | Variable | Paleta de colores RGBA para el bloque |
| PAL_XCANAL | `0x05` | Variable | Paleta con canal alpha separado |
| RAW | `0x7F` | 256 bytes | Datos RGBA sin comprimir (64×4) |
| SPAN | `0x7E` | Variable | Filas idénticas fusionadas |
| BLOCK_END | `0x80` | 0 bytes | Terminador de archivo |

El archivo termina con un bloque `BLOCK_END`.

## 4. Especificación del Lenguaje .herm

Ver `04-guia-uso.md` para sintaxis completa.

### 4.1 Gramática Formal (BNF simplificada)

```
program      ::= scene_block (import_stmt | material_block | node_block | def_block | instance_stmt)*
scene_block  ::= "scene" STRING? "{" scene_body "}"
scene_body   ::= axes_decl camera_decl? background_decl?
axes_decl    ::= "axes" "{" "X" ":" INT "Y" ":" INT "W" ":" INT "}"
camera_decl  ::= "camera" "{" camera_field* "}"
material_block ::= "material" STRING "{" material_field* "}"
node_block   ::= "node" STRING? "{" node_body "}"
def_block    ::= "def" STRING ("(" IDENT ("," IDENT)* ")")? "{" node_body "}"
instance_stmt ::= "instance" STRING ("(" expr ("," expr)* ")")? instance_overrides?
import_stmt  ::= "import" STRING
```

### 4.3 Sustitución de Parámetros en `def`

Cuando un `def` tiene parámetros y un `instance` suministra argumentos:

1. El resolvedor construye un mapa `{param → arg_text}` para cada argumento.
2. Recorre el árbol SDF del `def` y sustituye textualmente cada ocurrencia del nombre del parámetro por el texto del argumento, respetando límites de palabra (no sustituye dentro de identificadores más largos).
3. El nodo resultante se resuelve como un nodo SDF normal (no como INSTANCE).
4. Las expresiones con `w`/`t` no se ven afectadas — permanecen como expresiones para evaluación por fotograma.

Esto permite escribir definiciones genéricas reutilizables:

```
def "tornillo"(largo, radio) {
    sdf: cylinder(radio, largo);
    material: "metal";
    position: [0, largo/2, 0];
}

instance "tornillo"(0.8, 0.05);
```

### 4.3 Sustitución de Parámetros en SDFs

Los parámetros de primitivas SDF ahora aceptan expresiones completas, no solo constantes. El flujo es:

1. **Parser**: la expresión textual se captura completa (e.g., `r * 0.5 + 0.1`) en un `ExprNode`.
2. **Resolver**: `toExpr()` intenta parsear como número. Si falla, guarda el texto como `Expr{ is_expr=true, expression="r * 0.5 + 0.1" }`.
3. **RIH**: la expresión se serializa como `{ "expr": "r * 0.5 + 0.1" }` en JSON, o como string offset en binario.
4. **Frame evaluation**: `evaluateFrame()` llama a `evalExpression()` para cada param `is_expr=true`, reemplazando `w`/`t` por el fotograma actual.
5. **Render**: el renderer usa `params[i].constant` (ya evaluado).

Esto cierra el círculo: cualquier lugar donde se espera un número en la sintaxis puede recibir una expresión analítica, incluyendo variables de tiempo (`w`, `t`), parámetros de `def`, y operaciones aritméticas/funciones.

### 4.4 Convenciones Léxicas

- Palabras clave en **minúsculas**
- Comentarios: `//` línea, `/* */` bloque
- Identificadores: `[A-Za-z_][A-Za-z0-9_]*`
- Cadenas: `"entre comillas dobles"`
- Bilingüe: español e inglés (palabras clave equivalentes)
