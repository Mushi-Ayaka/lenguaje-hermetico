# Lenguaje Hermético

Lenguaje de descripción de escenas 3D/4D basado en **campos de distancia firmados (SDF)**. Describe escenas completas con iluminación, materiales PBR, animación, y volumen rendering — todo sin mallas ni polígonos, puramente con geometría procedural y operaciones algebraicas.

## Pipeline

```
.herm ──→ herm.exe ──→ .ont + .obs ──→ motor-grafico (runtime Vulkan)
                 ├──→ .rih (JSON) / .rib (binario)   [IR intermedio]
                 ├──→ --render flag ──→ render directo ──→ .png (vía stb_image_write)
                 └──→ --ont flag ──→ formato ontológico binario (.ont) + metadata (.obs)

(legacy) .herm ──→ herm.exe ──→ .rih ──→ operador_tb ──→ .tb ──→ render.exe ──→ .png
```

## Componentes

### Compilador (`herm/`)

| Módulo | Propósito |
|--------|-----------|
| `herm_lexer` | Tokeniza archivos `.herm` (soporta keywords bilingües español/inglés) |
| `herm_parser` | Análisis sintáctico: nodos SDF, materiales, luces, def/instance, let, import, fn |
| `herm_ast` | Árbol de sintaxis abstracta: primitivas, operaciones, escena, materiales, luces |
| `herm_resolver` | Resuelve referencias: def→instance, let/fn→expansión textual, materiales |
| `herm_rih` | Emisor de RIH (Representación Hermética Intermedia) en JSON/binario |
| `herm_ont` | Emisor del formato ontológico binario `.ont` (BVH + bytecode SDF + materiales) |
| `herm_obs` | Emisor de metadata `.obs` (cámara, luces, timeline, background) |
| `herm_compile` | API de compilación para el motor (`compileToRih`/`compileFromString`) |
| `herm_eval` | Evaluador de expresiones SDF con contexto x,y,z,w,t |
| `herm_render` | Renderizador directo: ray marching + Blinn-Phong multi-light |
| `main.cpp` | CLI: `herm input.herm [-o out] [--ont] [--render output.png]` |

### Librerías (`libreria/`)

55 librerías estándar (`std_*.hm`) del árbol ontológico (matemáticas → física → química → biología → geociencias → astrofísica). Las 55 compilan. `materiales.hm` está archivado en `archived/libreria/` (DL-27). Ver `libreria/README.md` y `docs/11-protocolo-librerias.md`.

### Backend independiente (`operador_tb/`) — LEGACY

Pipeline antiguo `.rih → .tb → render.exe`. Sin mantenimiento; el consumo activo lo hace `motor-grafico` vía `.ont`.

| Módulo | Propósito |
|--------|-----------|
| `rih_reader` | Lector de RIH JSON |
| `json_reader` | Parser JSON genérico |
| `renderer` | Ray marching + shading (Blinn-Phong v2 con multi-light) |
| `tb_writer` | Escritura en formato .tb (Tensor Blocks) para el pipeline de Lienzo y renderizado |

### Contratos C++ (`contrato/`)

Cabeceras que definen la estructura de datos compartida:

| Archivo | Define |
|---------|--------|
| `rih.h` | RIH completo: versión, escena, materiales, nodos, luces |
| `herm.h` | Estructuras del lenguaje: ejes, parámetros, SDF, def/instance |
| `material.h` | Propiedades de material (0% implementado — pendiente) |
| `scene.h` | Estructura de escena (0% implementado — pendiente) |
| `sdf.h` | Tipos SDF (0% implementado — pendiente) |

### Librería legacy (`archived/libreria/`)

`materiales.hm` — catálogo pre-tensor de ~55 materiales PBR, **archivado y no compilable** (usa `ior`/`transmission`, rechazados por el parser actual). Se conserva como referencia de valores; el camino activo es definir materiales con `tensor` en la escena:
- **Metales**: oro, plata, cobre, hierro, aluminio, latón, bronce, titanio, cromo, platino, níquel
- **Dieléctricos**: vidrio, agua, diamante, hielo, cristal
- **Orgánicos**: madera roble/pino/cerezo, cuero, caucho, piel, cera
- **Piedra**: piedra, concreto, ladrillo, mármol blanco/verde, granito, pizarra
- **Emisivos**: luz blanca, sol, neón rojo/azul/verde, luz cálida/fría
- **Plásticos**: barniz, mate, acrílico, esmalte

## Lenguaje .herm

### Primitivas SDF

| Primitiva | Parámetros |
|-----------|-----------|
| `sphere` | radio |
| `box` | ancho, alto, profundidad |
| `cylinder` | radio, altura |
| `capsule` | radio, largo |
| `torus` | radio_mayor, radio_menor |
| `plane` | normal |
| `cone` | radio_base, altura |
| `rounded_box` | ancho, alto, prof., radio_esquina |
| `octahedron` | tamaño |
| `tesseract` | tamaño (4D) |
| `sphere4D` | radio (4D) |

### Operaciones

| Operación | Descripción |
|-----------|-----------|
| `union(a, b)` | Combinar sólidos |
| `subtract(a, b)` | Restar B de A |
| `intersect(a, b)` | Intersección |
| `smooth_union` / `smooth_subtract` / `smooth_intersect` | Versiones suavizadas con radio k |
| `repeat` | Repetir en ejes con espaciado |
| `twist` | Torcer alrededor de un eje |
| `elongate` | Estirar un volumen |
| `displace` | Desplazar con ruido/expresión |

### Características

- **Def/Instance**: `def nombre(params) { }` / `instance nombre(args)`
- **Variables `let`**: Expansión textual en tiempo de compilación
- **Materiales PBR**: base_color, roughness, metallic, emission, ior, transmission, opacity
- **Múltiples luces**: directional y point
- **Animación**: dimensión `w` como tiempo (4D geometry)
- **Volume rendering**: Modo volumen con integración Beer-Lambert
- **Keywords bilingües**: español e inglés en todos los constructos
- **Expresiones**: variables (x,y,z,w,t), constantes (pi, e), funciones (abs, sin, cos, sqrt, pow, max, min, clamp, lerp)

## Quick Start

```batch
cd herm
build.bat                          # Compila herm.exe

# Compilar .herm → .rih
herm.exe ..\ejemplos\bodegon.herm -o salida.rih

# Compilar y renderizar directamente
herm.exe ..\ejemplos\bodegon.herm --render salida.png

# Pipeline completo con operador_tb
cd ..\operador_tb
build.bat                          # Compila operador_tb.exe
operador_tb.exe ..\ejemplos\bodegon.rih -o salida.tb
```

## Ejemplos (`ejemplos/`)

| Archivo | Descripción |
|---------|-----------|
| `bodegon.herm` | Bodegón clásico (tablero, jarron, esfera, patas) |
| `torre.herm` | Torre con repetición |
| `instancias.herm` | Múltiples instancias de un def |
| `teclado.herm` | Teclado con teclas individuales y sombras |
| `animada.herm` | Escena animada (w como tiempo) |
| `luces.herm` | Múltiples fuentes de luz directional + point |
| `volumen.herm` | Volumen rendering (nubes, humo) |
| `sdf_*.herm` | Expresiones SDF en parámetros y animación |

## Prototipo Python (`prototipo/`)

Implementación independiente del concepto "cosmos tensorial":
- `Tensor1x8` — tensor de 8 componentes
- `Field` — grid sparse infinito basado en diccionario
- `Rule` — sistema de reglas de campo con prioridades
- `Engine` — motor de simulación con caché temporal

## Estructura del Proyecto

```
Lenguaje Hermetico/
├── herm/           # Compilador principal
├── operador_tb/    # Backend de renderizado y TB writer
├── contrato/       # Cabeceras de contrato (RIH, Herm, Material, Scene, SDF)
├── libreria/       # Biblioteca de materiales .hm (~40 materiales)
├── ejemplos/       # Escenas de ejemplo (.herm + .rih generados)
├── prototipo/      # Prototipo Python del modelo tensorial
├── docs/           # 13 documentos de documentación
├── 3rdparty/       # stb_image_write.h
└── archived/       # Documentación antigua y outputs históricos
```

## Documentación

`docs/` incluye 13 documentos: fundamentos matemáticos, arquitectura del sistema, especificaciones técnicas, guía de uso, implementación, métricas, complejidad, changelog (v0.1→v0.14), historial de decisiones, contrato ontológico, deuda técnica, y protocolo para librerías científicas.

## Requisitos

- MSVC (Build Tools o Visual Studio 2022+)
- C++17
- Windows x64
- Python 3.x (solo para el prototipo)
