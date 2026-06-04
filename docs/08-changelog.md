# Changelog

## 2026-06-04 — v0.14 — Migración de PBR a Plano Tensorial 1x8

### Añadido
- **Plano Tensorial 1x8 (`tensor`)**: Los materiales ahora se expresan como un tensor de 8 dimensiones `[x, y, z, w, r, g, b, a]`, eliminando las propiedades estáticas aisladas del PBR tradicional (`base_color`, `roughness`, `metallic`, `emission`, etc.).
- **Evaluación óptica dinámica**: El sistema de sombreado en `herm_render.cpp` y el runtime nativo `operador_tb/renderer.cpp` ahora evalúan el color base `r, g, b, a` y la emisión volumétrica extrayendo los componentes de los índices 4..7 del tensor directamente del material para cada punto `(x,y,z,w)` en el espacio-tiempo.
- **Transmisión interactiva en tiempo de compilación**: El compilador (`herm_resolver.cpp` y `herm_rih.cpp`) extrae e incrusta el comportamiento del tensor (incluyendo expresiones dependientes del tiempo o el espacio) emitiendo matrices genéricas para el backend.
- **Sintaxis unificada de material**: La inicialización en los scripts `.herm` se simplificó de un listado manual de propiedades al uso de la palabra clave `tensor: [...]`. (Ej. actualizado en `ejemplos/volumen.herm`).

### Eliminado
- **Propiedades de dominio estáticas (PBR) nativas**: Se removieron variables físicas impuestas al motor subyacente. Los modelos de iluminación avanzados (Fresnel, GGX, reflectancia) ya no están codificados en C++, allanando el camino para que emerjan orgánicamente desde código nativo del lenguaje en librerías (`.hm`).

## 2026-06-04 — v0.13 — Deuda técnica, auditoría de sobreingeniería, simplificación

### Diagnosticado
- **PBR y BVH no son features del lenguaje** — son conocimiento de dominio (óptica, biología) que deben vivir en librerías `.hm`, no como C++ hardcodeado.
- **Falta `x, y, z` en el evaluador** — el evaluador no conoce el punto del rayo, por eso las SDFs tienen que estar hardcodeadas en C++.
- **Falta `hash()` determinista** — sin él no hay formas naturales ni generación por semilla.
- **Materiales homogéneos** — no hay composición volumétrica de múltiples substancias en el mismo cuerpo.
- **Sin operadores de forma natural** — `fragmento`, `erosión`, `cristalización` no existen.

### Auditoría de sobreingeniería
- **Pipeline MLIR/GTE/PTR/CTB**: estaba documentado como parte del core cuando **es un proyecto aparte** (`Lienzo y renderizado/`). Docs actualizados.
- **Formato `.rib` binario**: `writeRihBinary()` de 167 líneas, pero **no hay `readRihBinary()`** — formato huérfano.
- **Flag `--tb`**: parseado en `main.cpp` pero **nunca ejecutado**.
- **3 headers de contrato vacíos**: `material.h` (24 funciones declaradas, 0 implementadas), `sdf.h` (12 funciones, 0 implementadas), `scene.h` (8 funciones, 0 implementadas). ~260 líneas de código muerto.
- **Dos renderers duplicados**: `herm_render.cpp` y `renderer.cpp` ~95% idénticos (~1500 líneas).
- **Bilingüe duplica lexer**: ~340 líneas para keywords EN/ES cuando una tabla bidireccional bastaría.
- **14 librerías ontológicas planeadas** en `contrato-ontologico.md`: solo `materiales.hm` existe.

### Simplificación
- **No hay formato `.ether`** — las librerías son archivos `.hm` que ya existen. `import` + `def` basta.
- **No se necesita nuevo parser** — solo `x, y, z` en el evaluador y expresión SDF como texto en RIH.
- **Las primitivas SDF se mudan de C++ a `std_sdf.hm`**: `def "sphere"(r) { length(p) - r }`.

### Añadido
- **`docs/10-deuda-tecnica.md`**: reescrito con el enfoque simplificado.
- **`docs/contrato-ontologico.md`**: reescrito — eliminadas 14 librerías, eliminadas referencias a MLIR/GTE/PTR/CTB.

### Cambiado
- **`docs/02-arquitectura.md`**: simplificado — eliminado pipeline MLIR, flags `--gte` y `--tb`.
- **`docs/00-indice.md`**: actualizado con índice corregido.
- **`docs/09-historial.md`**: §11 actualizado con auditoría y roadmap simplificado.

---

## 2026-06-03 — v0.12 — Variables let (expansión en compilación)

### Añadido
- **Variables `let`/`sea`**: definición de constantes con nombre en el cuerpo del archivo. Se expanden textualmente durante la resolución — cero impacto en runtime.
- **Sintaxis**: `let NOMBRE = expresión;`. Soporta cadenas entre lets (`let A = B + 1; let B = 2;` se resuelve iterativamente).
- **Compatibilidad con defs parametrizadas**: las variables `let` se aplican después de la sustitución de parámetros en `cloneSubstSdf()`, permitiendo combinaciones como `def "x"(r) { sdf: sphere(r * ESCALA); }`.
- **Built-in prioridad**: `pi`, `e`, `w`, `t`, `x`, `y`, `z` son built-ins del evaluador y no pueden ser reemplazados por `let`.
- **Sintaxis bilingüe**: `let`/`sea`.
- **Escenas de prueba**: `ejemplos/def_let.herm` (`let TAU = pi * 2; sphere(RADIO);`), `ejemplos/def_let2.herm` (let + def params + instancias).

### Cambiado
- **`herm_lexer.h/.cpp`**: nuevos tokens `LET`, `SEA`.
- **`herm_ast.h`**: `AstLet { name, value }`, `AstProgram::lets`.
- **`herm_parser.h/.cpp`**: `parse_let()` y dispatch en `parse()`.
- **`herm_resolver.cpp`**: `let_table` en estado del resolvedor, recolección y resolución de cadenas en `resolve()`, aplicación en `toExpr()`, `evalExpr()` y `cloneSubstSdf()`.
- **Documentación actualizada**: guía de uso (§15 Let), changelog, historial.

### Importancia ontológica
- `let` refuerza el principio hermético: las constantes se resuelven en compilación, no hay estado mutable en runtime.
- En escenas densas (ej. mundos tipo Minecraft con miles de celdas), cada `let` usado en 10 referencias ahorra 10 lookups por celda. El compilador absorbe el costo, el runtime no paga nada.
- La expansión textual vía `substExprText()` (mismo mecanismo que defs parametrizadas) garantiza que no hay indirección en el RIH — el resultado ya llega expandido a constantes o expresiones con `w`.

### Métricas
- def_let.herm (2 let, 2 nodos, 400×300): ~200ms (idéntico a sin let)
- def_let2.herm (4 let + def params, 4 nodos, 400×300): ~145ms

---

## 2026-06-03 — v0.11 — Múltiples luces (directional / point)

### Añadido
- **Múltiples luces**: nuevo tipo de nodo `light`/`lumbre` en el AST. Soporta `type` (`directional`/`point`), `direction`, `position`, `color`, `intensity`, `falloff`.
- **RIH `Light`/`LightType`**: nuevas estructuras en `contrato/rih.h`. `Rih::lights` como vector de luces. Serialización JSON + binaria.
- **Renderer multi-luz**: ambos renderers (`herm_render.cpp`, `operador_tb/renderer.cpp`) iteran sobre la tabla de luces acumulando contribuciones por luz. Sombra ray solo para luces direccionales. Atenuación por distancia para luces puntuales.
- **Default backward‑compatible**: si no hay luces definidas, se usa una luz direccional por defecto `(0.3, 0.8, 0.5)`, manteniendo la compatibilidad con escenas existentes.
- **Sintaxis bilingüe**: `light`/`lumbre`, `type`/`tipo`, `direction`/`direccion`, `intensity`/`intensidad`, `falloff`/`caida`.
- **Documentación actualizada**: guía de uso (§14 Luces), changelog, historial.

### Cambiado
- **`contrato/rih.h`**: `Light` struct, `LightType` enum, `LightRecord` binario, `num_lights` en header binario.
- **`herm_lexer.h/.cpp`**: nuevos tokens: `LIGHT`, `LUMBRE`, `TYPE`, `TIPO`, `POINT_LIGHT`, `PUNTUAL`, `DIRECTION`, `DIRECCION`, `INTENSITY`, `INTENSIDAD`, `FALLOFF`, `CAIDA`.
- **`herm_ast.h`**: `AstLight` struct, `AstProgram::lights` vector.
- **`herm_parser.h/.cpp`**: `parse_light()` y dispatch en `parse()`.
- **`herm_resolver.h/.cpp`**: `resolveLight()` y loop en `resolve()`.
- **`herm_rih.cpp`**: JSON + binary serialización de luces.
- **`operador_tb/rih_reader.cpp`**: `parseLight()` y parsing en `loadRih()`.

### Ejemplos
- `ejemplos/luces.herm`: dos luces direccionales + una puntual sobre esfera metálica y plano difuso.

### Métricas
- luces.herm (3 luces, 2 nodos, 400×300): ~120ms render+emit (marginal vs 1 luz, el shading es ~5% del tiempo total)

---

## 2026-06-03 — v0.10 — Expresiones en SDFs, animación por parámetro

### Añadido
- **Expresiones en parámetros SDF**: `SdfNode::params` cambió de `f32[4]` a `Expr[4]`. Ahora se puede escribir `sphere(r * 2)`, `cylinder(h * 0.5, r + 0.1)`, `sphere(0.65 + sin(w * 2) * 0.35)` — cualquier expresión analítica es válida como parámetro de primitiva SDF.
- **Evaluación por fotograma**: `evaluateFrame()` ahora procesa las expresiones en params SDF igual que las transformaciones. Animaciones como `sphere(w * 0.2 + 0.3)` funcionan directamente.
- **Escenas de prueba**: `ejemplos/sdf_expr.herm` (expresión constante `1.5 * 0.5`), `ejemplos/sdf_expr_param.herm` (expresión con def parametrizada: `rad * 0.5`), `ejemplos/sdf_anim_expr.herm` (radio pulsante con `sin(w * 2)`).

### Cambiado
- **`contrato/rih.h`**: `SdfNode::params[4]` de `f32` a `Expr`. `SdfRecord::params` (binario) a `union { f32 constant; u32 string_offset; }`.
- **`herm_resolver.cpp`**: `resolveSdfTree()` usa `toExpr()` en vez de `evalExpr()` — las expresiones no constantes se preservan como `Expr` con `is_expr=true`.
- **`herm_rih.cpp`**: serialización JSON usa `jsonExpr()` para params. Serialización binaria escribe constantes o string offsets. `evaluateFrame()` añadió `evalNodeSdfParams()`.
- **`herm_render.cpp` + `operador_tb/renderer.cpp`**: todas las referencias `node.params[i]` → `node.params[i].constant`.
- **`operador_tb/rih_reader.cpp`**: `parseSdfNode` usa `parseExpr()` para leer params desde JSON (soporta `{ "expr": "..." }` y números planos).

### Métricas
- sdf_expr.herm (2 nodos, 400×300): ~100ms render+emit
- sdf_anim_expr.herm (1 nodo, 6 frames, 400×300): 458ms total
- Todos los ejemplos existentes siguen funcionando sin cambios

---

## 2026-06-03 — v0.9 — Volumen rendering, defs parametrizadas, documentación completa

### Añadido
- **Modo `volume`**: integración Beer-Lambert en `herm_render.cpp` y `operador_tb/renderer.cpp`. Nuevos campos `vol_density`, `vol_emission_r/g/b`, `vol_material_id` en `EvalResult`. Función `rayMarchVol()` que integra densidad a pasos fijos y compone con color sólido.
- **Defs con parámetros**: sintaxis `def "name"(param1, param2) { ... }` e `instance "name"(arg1, arg2)`. Sustitución textual de parámetros en el AST del SDF. El resolvedor clona el árbol y sustituye antes de evaluar.
- **Documentación completa en `docs/`**: 11 documentos (00-indice a 09-historial + contrato-ontologico.md). Cubren fundamentos, arquitectura, especificaciones, guía de uso, implementación, métricas, complejidad, changelog, historial y contrato ontológico.
- **`docs/contrato-ontologico.md`**: documento raíz del modelo ontológico — define qué es correcto (keywords), qué falta (librerías), qué no pertenece al lenguaje.

### Cambiado
- **`EvalResult`** ahora incluye campos de volumen. `FlatNode` tiene `is_volume` y `mode`.
- **`operador_tb/renderer.cpp`**: sincronizado con `herm_render.cpp` — volumen, base_color como fallback de emisión, shadow.
- **`evalScene`** en ambos renderers: ahora también trackea densidad de volumen por nodo.
- **`escapeExpr`** y otros campos: preparados para expresiones en SDF params (actualmente f32).
- **Documentos actualizados**: todos los `docs/*.md` reflejan el estado actual del proyecto.

### Métricas
- volumen.herm (2 nodos, 400×300): 125ms render+emit
- Build ambos proyectos sin errores

---

## 2026-03-06 — v0.8 — Teclado, sombras, grupo compose, primitivas 4D

### Añadido
- **Escena teclado.herm**: teclado QWERTY completo (36 teclas + space + 2 shift) con animación "HOLA MUNDO!" usando expresiones `max(0, 1 - abs(w - N)) * 0.12`
- **Sombra (`shadowRay`)**: segundo ray march desde punto de impacto hacia la luz. Factor 0.3 cuando hay bloqueo. 32 pasos máx, 5.0 distancia máx.
- **SDF `sphere4d`**: hiperesfera 4D — radio aparente se reduce con w-offset
- **SDF `tesseract`**: hipercubo 4D — corte transversal = caja redondeada
- **SDF `twist`**: rotación del espacio alrededor de Y proporcional a y
- **SDF `elongate`**: estiramiento — resta half-extents de |p|
- **SDF `displace`**: desplazamiento por expresión analítica sin noise
- **flags `--render`**: pipeline completo `.herm → .png` (self-contained)

### Cambiado
- **Composición de grupos** (`flattenNodes`): `accumulate()` camina child→parent acumulando transforms; instancias heredan transforms del grupo
- **Build**: `/O2` en herm/build.bat y operador_tb/build.bat (23× más rápido)

### Corregido
- **Bug `Transform acc = {}`**: translate/rotate no se inicializaban en `accumulate()`

### Métricas
- bodegon (herm --render): 58s debug → 2.5s /O2
- operador_tb bodegon: 39s debug → 2.7s /O2
- teclado frame: ~27-31s a 800×600, 40 nodos

---

## 2026-03-05 — v0.7 — operador_tb standalone

### Añadido
- **`operador_tb/`**: backend independiente que lee RIH JSON y produce .tb
- **`json_reader.cpp/.h`**: parser JSON recursivo descendente
- **`rih_reader.cpp/.h`**: mapea JSON → estructuras `herm::Rih`
- **`tb_writer.cpp/.h`**: escribe formato .tb (bloques 8×8, P0, PAL_RGBA, RAW, BLOCK_END)
- **Pipelines completos**: `.herm → RIH → operador_tb → .tb → .png`

---

## 2026-03-04 — v0.6 — RIH, animación, resolución

### Añadido
- **RIH (Representación Intermedia Hermética)**: formato JSON para escena, materiales, nodos, SDFs, animación
- **Expresiones animadas**: sintaxis `{"expr": "sin(t*2)*3"}` en transformaciones
- **Resolvedor**: evalúa SDFs, herencia, animación

### Cambiado
- **Arquitectura**: unificación de compilador + resolvedor en un binario `herm.exe`
- **Salida**: RIH como formato de intercambio principal (JSON + binario .rib)

---

## 2026-03-03 — v0.5 — Parser .herm, sintaxis bilingüe

### Añadido
- **`herm_parser.cpp`**: parser completo del lenguaje .herm
- **Sintaxis bilingüe**: español/inglés para todas las palabras clave
- **`herm_lexer.cpp`**: lexer con soporte para comentarios, strings, números, vectores
- **Definiciones (`def`) e instancias (`instance`)**
- **Grupos con herencia de transformaciones**
- **Expresiones aritméticas** en tiempo de compilación
- **Materiales**: tabla global con propiedades PBR

### Ejemplos
- bodegon.herm, instancias.herm, torre.herm, animada.herm

---

## 2026-03-02 — v0.4 — Librería de materiales

### Añadido
- **`libreria/materiales.hm`**: 40+ materiales predefinidos
  - Metales: oro, plata, cobre, hierro, aluminio, latón, bronce, titanio, cromo, platino, níquel, acero
  - Dieléctricos: vidrio, agua, diamante, hielo, cristal
  - Orgánicos: maderas (roble, pino, cerezo), cuero, caucho, piel, cera
  - Piedra: piedra, concreto, ladrillo, mármoles, granito, pizarra
  - Emisivos: luz_blanca, sol, neones, luz_cálida, luz_fría
  - Plásticos: barniz, mate, acrílico, esmalte

---

## 2026-03-01 — v0.3 — Contratos y fundamentos

### Añadido
- **`contrato/`**: archivos de cabecera con tipos del lenguaje
  - `herm.h`: estructuras del compilador
  - `rih.h`: tipos de la RIH
  - `material.h`: propiedades de material
  - `scene.h`: estructura de escena
  - `sdf.h`: tipos de SDF
- **`Representacion hermetica.md`**: fundamentos matemáticos completos
- **`ARQUITECTURA.md`**: documento de arquitectura
- **`SINTAXIS.md`**: documento de sintaxis

---

## 2026-02-28 — v0.2 — Prototipo de render SDF

### Añadido
- Primitivas SDF: sphere, box, cylinder, torus, plane, cone, capsule, rounded_box
- Operadores SDF: union, subtract, intersect, smooth variants
- Sphere tracing con iluminación Blinn-Phong
- Pipeline básico .gte → .ptr → .ctp → .tb → .png

---

## 2026-02-27 — v0.1 — Concepto inicial

### Añadido
- Idea fundacional: matriz 1×8 como estado del universo
- Concepto de operadores P (Perturbación), I (Influencia), K (Aniquilación)
- Pirámide Física-Química-Biología
- Eje W como tiempo geométrico (universo de bloque)
- Modelo SDF para geometría implícita
- Colaboración con agentes externos para debate matemático
- Decisión: no a redes neuronales, no a vóxeles, no a ruido estadístico
