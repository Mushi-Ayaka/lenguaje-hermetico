# Lenguaje Hermético — Guía Completa de Sintaxis y Capacidades

**Última actualización:** 2026-10-16  
**Estado:** v0.18 — post Fase 1, pre Fase 2 (DL-14 fn con lets internos)

---

## 1. Estructura General

```herm
// Comentario de una línea
/* Comentario de bloque */

// Declaraciones en orden libre (excepto imports al inicio)
import "libreria/std_math_core.hm"

let PI = 3.14159;

def "esfera_r"(r) { sphere(r) }

material "mat_pared" {
  base_color: [0.8, 0.8, 0.8];
  roughness: 0.9;
}

scene "Mi Escena" {
  axes: [1920, 1080];
  background: [0.1, 0.1, 0.15];
  
  camera {
    position: [0, 2, -5];
    rotation: [0, 0, 0];
    fov: 60;
  }
  
  node "pared" {
    position: [0, 0, 0];
    sdf: box(10, 0.5, 10);
    material: "mat_pared";
  }
  
  node "columna" {
    position: [3, 1, 3];
    sdf: instance "esfera_r"(0.5);
    material: tensor: [0, 0, 0, 1, 1, 0, 0, 1];
  }
}
```

---

## 2. Palabras Clave (ES/EN)

| Español | English | Uso |
| --------- | --------- | ----- |
| `escena` | `scene` | Bloque raíz de escena |
| `nodo` | `node` | Objeto en la escena |
| `definicion` | `def` | Definición de SDF parametrizada |
| `instancia` | `instance` | Instancia de una definición |
| `funcion` | `fn` | Función matemática pura |
| `sea` | `let` | Constante/variable top-level |
| `importar` | `import` | Importar librería |
| `material` | `material` | Definición de material |
| `lumbre` | `light` | Fuente de luz |
| `camara` | `camera` | Cámara |
| `ejes` | `axes` | Resolución [ancho, alto] |
| `fondo` | `background` | Color de fondo |

---

## 3. Primitivas SDF

Todas reciben parámetros numéricos o expresiones:

```herm
sphere(r)                          // Esfera radio r
box(sx, sy, sz)                    // Caja semiejes
cylinder(r, h)                     // Cilindro radio r, altura h
capsule(r, h)                      // Cápsula radio r, altura semitotal
torus(R, r)                        // Toro radio mayor R, menor r
plane(offset)                      // Plano horizontal en y=offset
cone(r, h)                         // Cono radio r, altura h
rounded_box(sx, sy, sz, radius)    // Caja redondeada
octahedron(s)                      // Octaedro semilado s
tesseract(s)                       // Teseracto 4D semilado s
sphere4d(s)                        // Hipersfera 4D radio s
```

### Argumentos por nombre (no soportado aún — usar posición)

```herm
// sphere(2.0) — correcto
// sphere(radius: 2.0) — NO soportado
```

---

## 4. Operadores SDF

### 4.1 Binarios (dos SDFs)

```herm
union(a, b)                    // Unión d = min(da, db)
subtract(a, b)                 // Resta d = max(da, -db)
intersect(a, b)                // Intersección d = max(da, db)
smooth_union(a, b, k)          // Unión suave con radio k
smooth_subtract(a, b, k)       // Resta suave
smooth_intersect(a, b, k)      // Intersección suave
```

### 4.2 Unarios con parámetro

```herm
repeat(a, [spacing_x, spacing_y, spacing_z])  // Repetición periódica
twist(a, angle_rad)                            // Giro progresivo
elongate(a, [half_x, half_y, half_z])          // Elongación
displace(a, expr_func)                         // Desplazamiento por expresión
```

### 4.3 Composición

```herm
let forma = subtract(
  smooth_union(
    box(1, 1, 1),
    sphere(0.8),
    0.2
  ),
  cylinder(0.4, 2)
);
```

---

## 5. Expresiones

### 5.1 Variables Espaciales (solo dentro de expresiones)

| Variable | Significado |
| ---------- | ------------- |
| `x, y, z` | Coordenadas espaciales |
| `w` (o `t`) | Coordenada 4D / tiempo |
| `nx, ny, nz` | Normal (si disponible) |
| `vx, vy, vz` | Vector de visión |
| `lx, ly, lz` | Dirección de luz |

### 5.2 Operadores Aritméticos

```herm
+ - * / % ^
unario: -x
asociativos: ^ > * / > + -
```

### 5.3 Funciones del Evaluador CPU (herm_eval)

```herm
// Trigonométricas
sin(x), cos(x), tan(x), tanh(x)

// Algebraicas
abs(x), sqrt(x), floor(x), ceil(x), pow(x, y)

// Comparación/Selección (sin if!)
min(a, b), max(a, b)
clamp(x, lo, hi)
lerp(a, b, t) = mix(a, b, t)

// Muestreo recursivo (profundidad max 1)
sample(expr, dx, dy, dz)  // Evalúa expr en (x+dx, y+dy, z+dz)

// Constantes
pi, e, LN2, PLANCK
```

### 5.4 Funciones matemáticas y estado por backend

Desde el Paso 2.0b (2026-11-04) el evaluador del lenguaje (`evalExpression`, usado por `--render` y por los campos que conservan expresión) soporta:

```herm
step(edge, x)           // 0 si x < edge, 1 si x >= edge
smoothstep(a, b, x)     // Interpolación suave (Hermite)
exp(x), log(x), log2(x), log10(x)
acos(x), asin(x), atan(x), atan2(y, x)
sign(x)                 // -1, 0, 1
fract(x)                // parte fraccionaria
mod(x, y)               // módulo estilo GLSL: x - y*floor(x/y)
```

Más las que ya existían: `sin cos tan tanh abs sqrt floor ceil min max pow clamp lerp mix sample`, y las constantes `pi`, `e`, `LN2`, `PLANCK`.

**Notación científica:** el lexer acepta `1e-6`, `1E6` y `2.5e+3` desde 2.0c.2b (DL-33). Un exponente mal formado (`1e`, `1e+`) falla con error de expresión; no se interpreta como `1` seguido de un identificador.

**Importante — el pipeline `.ont` del motor todavía no las soporta.** El bytecode ONT (`OntOpcode` en `render/scene.h`) solo tiene `sin cos tan abs sqrt floor ceil min max pow clamp lerp mix mod` y aritmética. `step`, `smoothstep`, `exp`, `log*`, `acos`, `asin`, `atan*`, `sign`, `fract` y `tanh` **no tienen opcode**, así que el path GPU no puede representarlas. Extender el bytecode + sus 5 consumidores (VM, VM SIMD, GLSL, SPIR-V, JIT) es el Paso 2.0c.

Tampoco existen `length/dot/cross/normalize` como funciones nativas: las librerías las construyen a partir de `fn` en `.hm`.

---

## 6. `let` — Constantes Top-Level

```herm
let RADIO = 2.5;
let COLOR_ROJO = [1, 0, 0];
let POS = [0, 1, 0];

// Uso posterior
node "esfera" {
  sdf: sphere(RADIO);
  position: POS;
}
```

- **Alcance:** Global en el archivo (o librería importada)
- **Evaluación:** Constant-folded en compilación si es posible
- **No hay let local** excepto dentro de `fn` (DL-14, ver §7.2)

---

## 7. `fn` — Funciones Matemáticas Puras

### 7.1 Sintaxis Básica

```herm
fn "doble"(x) { x * 2.0 }
fn "dist2"(a, b) { sqrt(a*a + b*b) }
fn "clamp01"(v) { clamp(v, 0.0, 1.0) }
```

- **Paréntesis dobles:** `fn "nombre"(args) { expresión }`
- **Parámetros:** Nombres libres (no `x, y, z` fijos)
- **Cuerpo:** Una expresión (la última línea del bloque)
- **Retorno:** Implícito = valor de la expresión final

### 7.2 DL-14: `let` Internos (v0.17+)

```herm
fn "calcula"(a, b) {
  let suma = a + b;
  let producto = a * b;
  suma * 2.0 + producto  // ← retorno (última expresión)
}
```

- `let` locales permitidos desde v0.17
- **NO hay** `if`, `for`, ni comparadores
- La última expresión **debe** ser el retorno
- Error si la función solo tiene `let` sin expresión final

### 7.3 Uso de fn

```herm
fn "radio_dinamico"(t) { 1.0 + 0.5 * sin(t) }

node "esfera_viva" {
  sdf: sphere(radio_dinamico(w));
}
```

---

## 8. `def` — Definiciones SDF

### 8.1 Sintaxis

```herm
def "mi_forma"(r, h) {
  subtract(
    cylinder(r, h),
    sphere(r * 0.5)
  )
}
```

### 8.2 Uso con `instance`

```herm
node "objeto" {
  sdf: instance "mi_forma"(0.5, 2.0);
  position: [0, 1, 0];
}
```

- **Parámetros SDF:** Solo números/constantes (no `x, y, z` dinámicos)
- **Cuerpo:** Expresión SDF que usa los parámetros
- **`instance`:** Resuelve en tiempo de compilación

---

## 9. `material` — Materiales

### 9.1 Modelo Legacy (PBR)

```herm
material "madera" {
  base_color: [0.6, 0.4, 0.2];
  roughness: 0.8;
  metallic: 0.0;
  emission: [0, 0, 0];
  opacity: 1.0;
  blend_mode: "alpha";
}
```

### 9.2 Modelo Tensor (1×8) — Recomendado

```herm
material "rojo_sencillo" {
  tensor: [0, 0, 0, 1,  1, 0, 0, 1];
  //         x  y  z  w    r  g  b  a
}
```

**Layout del tensor:**

- `[0..3]` = `x, y, z, w` (mapeo espacial, normalmente `[0,0,0,1]`)
- `[4..6]` = `r, g, b` (color base)
- `[7]` = `a` (alpha/opacidad)

### 9.3 Material Inline en Nodo

```herm
node "objeto" {
  sdf: sphere(1);
  material: tensor: [0, 0, 0, 1,  0.2, 0.5, 0.8, 1];
}
```

---

## 10. `node` — Objetos en Escena

### 10.1 Campos

```herm
node "nombre" {
  position: [x, y, z];        // Traslación
  rotation: [rx, ry, rz];     // Rotación en radianes
  scale: [sx, sy, sz];        // Escala por eje
  
  sdf: <expresión SDF>;
  material: <nombre | tensor>;
  
  mode: "solid" | "volume";
  density_scale: 1.0;         // Solo para volume
  visible: true;              // Default: true
}
```

### 10.2 Modos

- `solid` (default): SDF sólido con shading PBR
- `volume`: Renderizado volumétrico (requiere `density_scale`)

---

## 11. `scene` — Escena

### 11.1 Estructura

```herm
scene "Nombre Opcional" {
  axes: [ancho, alto];        // Resolución de render
  background: [r, g, b];      // Color de fondo (0..1)
  
  camera {                    // Cámara (opcional, default auto)
    position: [x, y, z];
    rotation: [rx, ry, rz];
    fov: 60;                  // Campo de visión en grados
    aperture: 0.0;            // Apertura (profundidad de campo)
    focal_dist: 10.0;         // Distancia focal
  }
  
  light "sol" {               // Fuente de luz
    type: "directional";      // o "point"
    direction: [0, -1, 0];    // Para directional
    position: [10, 10, 10];   // Para point
    color: [1, 1, 1];
    intensity: 1.0;
  }
  
  // Nodos aquí...
  node "suelo" { ... }
}
```

### 11.2 Sin axes (default)

```herm
scene {
  background: [0, 0, 0];
  node "esfera" {
    sdf: sphere(1);
  }
}
```

---

## 12. `import` — Librerías

### 12.1 Sintaxis

```herm
import "libreria/std_math_core.hm"
import "std_math_core.hm"       // Búsqueda en cwd/libreria/
```

### 12.2 Extensiones

- **Archivos fuente:** `.hm` (Hermetico Module)
- **NO** usar extensión en el import (se agrega automáticamente)
- **Búsqueda:** Directorio del archivo fuente → `cwd/libreria/` → `cwd/`

### 12.3 Ejemplo

```herm
import "std_math_core"

let r = 2.0;
node "esfera" {
  sdf: sphere(r);
}
```

---

## 13. CLI — Compilación y Ejecución

### 13.1 Build del CLI

```bash
cd lenguaje-hermetico/herm
build.bat
```

Output: `lenguaje-hermetico/build/herm.exe`

### 13.2 Flags

| Flag | Output | Descripción |
| ------ | -------- | ------------- |
| *(default)* | `.rih` (JSON) | RIH JSON legible |
| `-o <prefix>` | Cambia prefijo | Ej: `-o salida` → `salida.rih` |
| `--bin` | `.rih` binario | Formato binario compacto |
| `--ont` | `.ont` + `.obs` | Ontología (pipeline motor) |
| `--render` | `.png` | Render directo a imagen (CPU) |
| `--gte` | `.gte` | Graphics Test Environment |
| `--tb` | *(flag)* | Tensor bytecode (legacy) |
| `--frames a-b` | Múltiples | Animación: frames a..b |

### 13.3 Ejemplos

```bash
# Compilar a RIH JSON
herm.exe escenas/mi_escena.herm

# Render directo a PNG (1024×576, CPU)
herm.exe escenas/mi_escena.herm --render

# Generar ontología para el motor
herm.exe escenas/mi_escena.herm --ont -o build/mi_escena

# Compilar rango de frames (animación en W)
herm.exe escenas/anim.herm --ont --frames 0-29
```

---

## 14. Pipeline del Motor (Vulkan)

Para usar en el motor gráfico (motor-grafico):

```bash
# 1. Compilar escena a .ont
herm.exe escenas/mi_escena.herm --ont

# 2. Reconstruir libherm (si tocaste herm/*.cpp)
cd motor-grafico
build_libherm.bat

# 3. Correr visor
build\visor.exe build\mi_escena.ont
```

---

## 15. Tests

### 15.1 Test Manual Rápido

```bash
# Compilar una librería (debe dar OK)
herm.exe libreria\std_math_core.hm --ont

# Renderizar una escena de prueba
herm.exe escenas\01_bodega_baja.herm --render
```

### 15.2 Tests Unitarios del Motor

```bash
cd motor-grafico/tests
build_f0_test.bat    # 24 tests pipeline base
build_f1_test.bat    # 10 tests RIH optimizer
build_f3_test.bat    # 16 tests shader generation
```

Output en `build/`:

- `test_f0.exe` → 24/24 PASS
- `test_f1.exe` → 10/10 PASS
- `test_f3.exe` → 16/16 PASS

### 15.3 Tests de Librerías

```bash
cd motor-grafico/tests
# Correr harness completo (verifica compilación de 55 libs)
powershell -File test_librerias.ps1
```

### 15.4 Test DL-14 (fn con lets)

```bash
herm.exe libreria\tests\test_dl14_fn_lets.herm --ont
# Esperado: OK 0 nodos, 0 materiales (solo compila fn)
```

---

## 16. NO HAZ — Limitaciones Conocidas

### 16.1 NO hay `if`

```herm
// MAL
if (x > 0) { 1.0 } else { -1.0 }

// BIEN (step de GLSL, solo GPU)
step(0, x) * 2.0 - 1.0

// BIEN (sin dependencia GPU)
sign_approx(x) = step(0.0, x) * 2.0 - 1.0
```

### 16.2 NO hay `for`

```herm
// MAL
for (i, 0, 10) { ... }

// BIEN (unroll manual)
let suma = 0;
// fn "sumatoria"(n) { ... }  // no hay loops, usar recursión limitada
```

### 16.3 NO hay comparadores de sintaxis

```herm
// MAL
if (a < b) { ... }
if (x == 0) { ... }
if (a && b) { ... }

// BIEN
min(a, b)              // equivalente a "el menor"
abs(x) < 0.001         // no existe, usar: x*x < 0.000001
step(0.5, cond)        // cond booleana → 0 o 1
```

### 16.4 `let` en nodo = Error

```herm
// MAL (let solo top-level o en fn)
node "x" {
  let r = 1.0;        // ← ParseError
  sdf: sphere(r);
}

// BIEN
let r = 1.0;           // top-level
node "x" {
  sdf: sphere(r);
}
```

### 16.5 Referencia Circular en `let` = Filtra Identificador Libre

```herm
// MAL (no cuelga, pero produce comportamiento indefinido)
let a = b * 2;
let b = a + 1;

// Error esperado: "undefined identifier" en runtime
```

---

## 17. Estado de las Librerías (v0.18)

- **Total:** 55 librerías `.hm`
- **Compilan OK:** 35 (64%)
- **Fallan por sintaxis:** 20 (36%)

### 17.1 Errores Comunes en Libs

| Error | Cantidad | Ejemplo Fix |
| ------- | ---------- | ------------- |
| `for` loop | 2 | Unroll manual con `def` + recursión |
| `if/else` | 3 | `step()` o `mix()` algebraico |
| Keyword como identificador | 8 | Renombrar (`alpha` → `alpha_val`) |
| `let` en nodo | 3 | Mover a top-level |
| Otros | 4 | Caso por caso |

### 17.2 Verificar Libs

```bash
# Verificar una librería específica
herm.exe libreria\std_geometria_base.hm --ont

# Esperar "OK" — si da error, leer mensaje
# "Error de sintaxis: se esperaba X pero se encontro Y en línea N"
```

---

## 18. Capacidades del Lenguaje

### 18.1 Sí — Soportado

- [x] Primitivas SDF 3D/4D (11 tipos)
- [x] Operadores booleanos SDF (union/subtract/intersect)
- [x] Operadores suaves (smooth_*)
- [x] Transformaciones (repeat/twist/elongate/displace)
- [x] Definiciones parametrizadas (`def`)
- [x] Funciones matemáticas puras (`fn`)
- [x] `let` top-level + `let` interno en `fn` (DL-14)
- [x] Materiales tensor (1×8) + legacy PBR
- [x] Luces direccionales y puntuales
- [x] Cámara con FOV/aperture/focal_dist
- [x] Animación en coordenada W (frames)
- [x] Importación de librerías (`.hm`)
- [x] Evaluación CPU + GPU (GLSL)
- [x] Pipeline ONT → motor Vulkan
- [x] Render directo a PNG (CPU)

### 18.2 NO — No Soportado

- [ ] `if` / `else`
- [ ] `for` / `while` loops
- [ ] Comparadores `<`, `>`, `==`, `&&`, `||`
- [ ] `let` local fuera de `fn`
- [ ] Clases / structs
 [ ] Arrays / listas (solo `[a, b, c]` como vector/matriz)
- [ ] Strings en tiempo de ejecución (solo identificadores)
- [ ] Métodos OO
- [ ] Manejo de errores (try/catch)
- [ ] Concurrencia / threads

---

## 19. Ejemplo Completo

```herm
// Escena completa con materiales, luces, cámara y animación
import "std_math_core"

let RAYO = 0.15;
let BASE_R = 1.2;

material "metal" {
  tensor: [0, 0, 0, 1,  0.7, 0.7, 0.8, 1];
}

material "vidrio" {
  tensor: [0, 0, 0, 1,  0.3, 0.6, 0.9, 0.3];
}

def "tornillo"(r, h) {
  subtract(
    cylinder(r, h),
    box(r * 2, r * 0.5, r * 0.3)
  )
}

fn "radio_anim"(t) {
  let base = 1.0;
  let pulso = 0.3 * sin(t * 2.0);
  base + pulso
}

scene "Demo" {
  axes: [1280, 720];
  background: [0.05, 0.05, 0.1];
  
  camera {
    position: [0, 3, -8];
    rotation: [0.3, 0, 0];
    fov: 45;
  }
  
  light "key" {
    type: "directional";
    direction: [-0.5, -1, -0.3];
    color: [1, 0.95, 0.9];
    intensity: 1.2;
  }
  
  light "fill" {
    type: "point";
    position: [5, 3, -2];
    color: [0.3, 0.5, 0.8];
    intensity: 0.4;
  }
  
  node "suelo" {
    position: [0, -0.5, 0];
    sdf: box(10, 0.1, 10);
    material: "metal";
  }
  
  node "columna" {
    position: [0, 1.5, 0];
    sdf: instance "tornillo"(0.3, 3.0);
    material: "vidrio";
  }
  
  node "esfera" {
    position: [3, 1, 2];
    sdf: sphere(radio_anim(w));
    material: tensor: [0, 0, 0, 1,  1, 0.5, 0.2, 1];
  }
}
```

---

## 20. Recursos Adicionales

- **Docs pipeline:** `motor-grafico/CONTEXT.md`
- **Changelog:** `lenguaje-hermetico/docs/08-changelog.md`
- **Decisiones:** `lenguaje-hermetico/DECISION_LOG.md`
- **Librerías:** `lenguaje-hermetico/libreria/`
- **Escenas test:** `motor-grafico/escenas/`

---

**Fin de la guía.** Para dudas específicas, revisar `docs/01-fundamentos.md` (matemáticas) o `docs/02-arquitectura.md` (pipeline).
