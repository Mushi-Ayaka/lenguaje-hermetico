# Historial de Iteraciones y Decisiones Arquitectónicas

## Lenguaje Hermético → Motor de Renderizado Ontológico (Éter → GTE)

Este documento registra la evolución del proyecto desde la idea inicial hasta el prototipo funcional, incluyendo los hallazgos críticos, las refutaciones y las decisiones que han dado forma al modelo matemático y a la arquitectura de compilación.

---

## 1. Idea inicial: "Jugar a ser Dios con matrices 1×8"

**Fuente:** `inicio.md` – investigación sobre un lenguaje declarativo para universos tensoriales.

**Concepto fundacional:**
- El universo se modela como un campo continuo de vectores de estado 1×8:  
  `v = [X, Y, Z, W, R, G, B, A]`
- En el vacío primordial, todo punto tiene `v = [0,0,0,0,0,0,0,0]`.
- **Crear** = perturbar el cero (operador `P`).
- **Modificar** = aplicar transformaciones matemáticas a los tensores que cumplen ciertas condiciones (`I`).
- **Eliminar** = multiplicar canales visuales por 0 (aniquilación `K`).
- Las formas emergen de **campos de distancia implícitos** (SDFs): `Φ(p) ≤ 0` → materia.
- El tiempo no es un reloj, es la cuarta dimensión `W` (universo de bloque). Movimiento = deformación de la línea de mundo en 4D.

**Decisión inicial:** No construir un motor de físicas tradicional, sino un **lenguaje de restricciones** donde el compilador (Resolvedor) resuelve un sistema de ecuaciones simultáneas.

---

## 2. Primer prototipo conceptual: "Onda expansiva" y la tentación de la simulación

**Fuente:** `prototipo con antigravity.md` – primer plano concreto.

**Propuesta inicial del prototipo:**
- Simular una onda que se expande en el espacio 2D usando ray marching sobre SDFs.
- Pretendía demostrar la viabilidad del modelo con un caso simple.

**Críticas recibidas (y autocríticas):**
- El prototipo todavía dependía de un bucle de actualización (simulación paso a paso), lo que reintroducía la dependencia del estado anterior y rompía el universo de bloque.
- No se distinguía claramente entre el **mundo ontológico** (leyes) y la **capa de observación** (cámara, tiempo de video).
- Se propuso inicialmente usar redes neuronales ligeras (Neural SDF) para acelerar el ray marching, pero se descartó porque:
  - Una red neuronal es una aproximación estadística, no determinista.
  - No se puede certificar la condición de distancia (||∇f|| = 1) para todos los puntos.
  - La salida puede variar entre GPUs y ejecuciones, violando la hermeticidad.
  - Se pierde la semántica ontológica (material_id, jerarquías).

**Decisión tomada:** Abandonar cualquier aproximación estadística. La SDF debe ser **exacta y analítica** en todo el dominio. La aceleración debe venir de estructuras de datos geométricas (BVH, AABB) y de la precomputación determinista, no de aproximaciones.

---

## 3. Colaboración con agente externo (Gemini): el debate sobre la aceleración

**Fuente:** `debate con geminis para mas ideas de calculo.md`

**Propuesta del agente externo:** Usar aritmética de intervalos + BVH 4D con macro‑saltos.

**Aportes clave:**
- La aceleración no debe buscar simplificar la SDF, sino **acelerar el vacío**.
- Precomputar en cada nodo del BVH la distancia mínima segura `d_min` y la constante de Lipschitz local `L`.
- En tiempo real, el rayo da macro‑pasos mientras `d_min > ε`, y solo cerca de la superficie evalúa el subgrafo ontológico exacto.
- Propuso **linearized BVH** almacenado en un array contiguo para GPU, con `skip_index` para navegación sin recursión.

**Críticas a esta propuesta:**
- El otro agente (yo, en el debate) señaló la **contradicción matemática del `min`**: Para una celda que contiene múltiples objetos, `d_min` es el mínimo de los `inf Φ_i` en la celda. Este valor puede ser pequeño aunque el punto actual del rayo esté lejos de todos ellos. La aritmética de intervalos no puede saber la posición relativa dentro de la celda.
- La aceleración solo es realmente efectiva en **vacío puro** o en celdas con un solo objeto. En regiones densas (bosques, ciudades) la ganancia es marginal.

**Decisión final:** Se mantiene la idea del BVH + AABB como **optimización de primer nivel**, pero se reconoce que el número de pasos en zonas densas sigue siendo un problema. Esto refuerza la necesidad de la **bisección afín sobre el parámetro del rayo** (interval ray tracing) como algoritmo de búsqueda de raíces en 1D, que elimina la dependencia de la constante de Lipschitz y reduce los pasos a un número fijo (ej. 20-30).

---

## 4. El modelo matemático definitivo (Representación Hermética)

**Evolución desde la matriz 1×8:**

| Versión | Características | Problemas detectados |
|---------|----------------|----------------------|
| Matriz plana `[X,Y,Z,W,R,G,B,A]` | Simple, pero mezcla dominio y codominio. | No permite distinguir forma de material. |
| `C(p) = s` con `s = [R,G,B,A]` | Función de mapeo directo. | Pierde la identidad de material. |
| `s_internal = (d, m, c, A, P)` | Separa distancia, material, color base, opacidad y propiedades. | Resuelve la pérdida de identidad. |
| `C(p)` devuelve estructura, `collapse()` produce `RGBA`. | Arquitectura limpia. | Requiere definir reglas de blending. |

**Decisiones sobre la química (materiales):**
- **Tabla global de materiales** (Material Table) indexada por `material_id`. Contiene: `base_color`, `roughness`, `metallic`, `emission`, `ior`, `transmission`, `opacity`.
- Cada entidad solo almacena `material_id` y `local_params` opcionales (tint, roughness_offset, density_factor).
- Esto permite cambios globales de apariencia sin recompilar geometría y facilita la reutilización.

**Pirámide Física – Química – Biología:**

| Capa | Responsabilidad | Datos |
|------|----------------|-------|
| **Física** | Forma, ocupación, transformaciones locales, bounding boxes | `(d, A, T_local, bbox, motion, E)` |
| **Química** | Materia, color, propiedades ópticas | `(material_id, base_color, roughness, metallic, emission, mode)` |
| **Biología** | Jerarquías, ensamblaje, herencia, animación | `(parent, children, T_hierarchy, rules)` |

**Modo de los objetos:**
- `solid`: superficie dura, opacidad binaria (`A = H(-d)`). La luz rebota o se absorbe.
- `volume`: medio participativo (niebla, fuego, agua), opacidad continua (`A = σ(d)`). La luz se atenúa.

**Reglas de solapamiento y empates:**
- **Sólido vs volumen:** el sólido tiene prioridad. Desplaza al volumen.
- **Volumen vs volumen:** composición volumétrica (Beer‑Lambert) o alpha blending.
- **Empate exacto (`|d1-d2| < ε`):** se resuelve por prioridad determinista del `material_id` (mayor ID gana). Elimina Z‑fighting.

**Tiempo como geometría:**
- El eje `W` es una coordenada más. Un objeto en movimiento define `X = f(W)`, `Y = g(W)`, etc.
- El renderizado es un corte transversal en `W = g(t)`.
- No hay simulación paso a paso; se puede saltar a cualquier `W` instantáneamente.

---

## 5. El contrato de compilación con GTE (backend)

**Evolución del papel de GTE:**

| Etapa | Rol de GTE | Problema |
|-------|------------|----------|
| Original | Lenguaje de alto nivel para expresiones por píxel. | Limitado a funciones sin estado, sin 3D, sin objetos. |
| Nueva arquitectura | Backend de compilación. Recibe slots con expresiones cerradas. | El Resolvedor debe traducir el mundo ontológico a esas expresiones. |

**El Resolvedor (compilador intermedio) debe:**
1. Tomar las declaraciones de entidades, reglas, materiales y jerarquías.
2. Resolver el sistema de ecuaciones implícitas (campos de distancia, acoplamientos).
3. Aplicar la cámara y la línea de tiempo (inyección de constantes).
4. Segmentar el espacio en slots (regiones rectangulares) para GTE.
5. Para cada slot, generar una expresión algebraica cerrada que devuelva `RGBA` en función de `(x, y, t)`.

**Formas de salida (mapeo de formatos):**

| Formato | Ejes en universo | Variables en GTE | Mecanismo |
|---------|------------------|------------------|------------|
| 2D + 1 frame | X,Y,Z continuos; W fijo | `x, y, t=0` | Proyección de cámara + corte en W. |
| 2D + secuencia | X,Y,Z continuos; W = t * factor | `x, y, t` | El tiempo de GTE controla W. |
| 3D + 1 frame | X,Y,Z continuos; W fijo | `x, y, t = Z` | Se mapea Z a t para renderizar rebanadas. |
| 3D + secuencia | X,Y,Z,W todos activos | `x, y, t` | Proyección 3D + tiempo. |

**Limitación aceptada:** GTE no tiene noción de SDFs ni de ray marching. Por tanto, el Resolvedor debe **hornear** (bake) la evaluación del ray marching en expresiones analíticas cerradas. Esto es posible mediante:
- **Aproximación polinómica** (Chebyshev, Taylor) para regiones suaves.
- **Pre‑renderizado a mapa de bits** (`load` en PTR) para regiones con alta frecuencia (bordes duros, texturas, etc.).

---

## 6. Decisiones sobre el prototipo actual

**Objetivo del prototipo:** Demostrar que el modelo ontológico puede generar código GTE válido para una onda que se expande.

**Elección:** La onda expansiva es una función **suave y continua**, por lo que puede expresarse como un polinomio o una fórmula analítica cerrada. Por tanto, **no es necesario pre‑renderizar a mapa de bits**. El Resolvedor puede generar directamente el slot GTE con una expresión matemática.

**Expresión de la onda (ejemplo):**
```
A = max(0, 1 - (distancia - radio_actual) / ancho_borde)
radio_actual = t * velocidad
```
Donde `distancia = sqrt(x^2 + y^2)` (suponiendo cámara ortográfica).

**Verificación:** El pipeline GTE → PTR → CTB → TB → PNG se completa sin errores, demostrando la viabilidad del puente ontológico.

---

## 7. Otras iteraciones descartadas (críticas y lecciones)

| Técnica propuesta | Por qué se descartó |
|-------------------|----------------------|
| **Grids uniformes de vóxeles** | Peso excesivo en 4D (maldición de la dimensionalidad). Rompe la elegancia del campo continuo. |
| **Regresión lineal por bloques** | Aproximación por trozos; rompe la continuidad C¹ de las normales (iluminación facetada). Además, requiere almacenar muchos coeficientes. |
| **Redes neuronales (Neural SDF)** | No determinista, pérdida de semántica, entrenamiento costoso, sin garantía de condición de distancia. |
| **Octree con polinomios lineales** | Mismo problema de discontinuidad en bordes de celda; normales saltan. |
| **Series de Taylor** | Error crece rápidamente lejos del punto de expansión; no es práctico para dominios grandes. |
| **PCA / reducción de dimensionalidad** | No aplicable directamente a campos de distancia no lineales. |
| **Ruido procedural sin ley subyacente** | Rompe el determinismo causal y la garantía de Lipschitz; solo se permite si es una perturbación analítica (ej. `sin(x)*cos(z)`). |

**Lección principal:** Cualquier aceleración debe mantener la **exactitud analítica** o, al menos, un error controlado y determinista. La pureza del modelo es más importante que la velocidad extrema.

---

## 8. Decisión: Parámetros de SDF como Expresiones

**Problema original:** Los parámetros de primitivas SDF (`sphere(r)`, `box(w, h, d)`, etc.) solo aceptaban constantes numéricas. Si se escribía `sphere(r * 2)` o `cylinder(h, r + 0.1)`, el resolvedor intentaba `std::stof` que fallaba y devolvía 0. Esto limitaba severamente la expresividad del lenguaje, especialmente combinado con defs parametrizadas — los parámetros solo podían usarse como valores directos, no en expresiones.

**Solución:** Cambiar `SdfNode::params` de `f32[4]` a `Expr[4]`, donde cada `Expr` puede ser:
- Una constante (`is_expr = false, constant = valor`): para números planos como `sphere(1.0)`.
- Una expresión (`is_expr = true, expression = "r * 2"`): para cualquier expresión analítica.

**Impacto arquitectónico:**
- **RIH (contrato)**: rotura menor — el formato JSON ya soportaba `{"expr": "..."}` (usado en transformaciones), solo había que aplicarlo a params SDF. El formato binario `.rib` cambia params de `f32[4]` a `union { f32; u32 }` con string offsets.
- **Resolvedor**: `evalExpr()` → `toExpr()` en `resolveSdfTree()`. Las expresiones no constantes se preservan sin evaluar en el RIH.
- **Frame evaluation**: `evaluateFrame()` gana `evalNodeSdfParams()` que recorre todos los SdfNodes y evalúa sus expresiones param por fotograma — exactamente igual que las transformaciones.
- **Backends**: ambos renderers cambian `params[i]` → `params[i].constant` (que tiene el valor evaluado).
- **Librerías ontológicas**: ahora un `def "tornillo"(largo, radio) { sdf: cylinder(radio * 0.5, largo); }` funciona correctamente porque `radio * 0.5` se preserva como expresión y se evalúa con el valor del argumento.

**Trade-off:** Los parámetros SDF ya no pueden ser `f32` en el RIH; ocupan más espacio (un `Expr` tiene ~40 bytes vs 4 de un `f32`). En la práctica, el RIH en memoria es pequeño (~KB) y el overhead es despreciable.

## 8.5 Decisión: Variables `let` — expansión en compilación

**Problema original:** Los usuarios escribían los mismos valores repetidos (radios, escalas, colores) en múltiples lugares del archivo .herm. No había manera de nombrar una constante y reutilizarla, lo que dificultaba mantener consistencia en escenas grandes.

**Solución:** Añadir `let nombre = expresión;` que define una constante con nombre. Durante la resolución, todas las referencias a ese nombre se sustituyen textualmente en las expresiones antes de evaluarlas o serializarlas en el RIH.

**Mecanismo:**
- El parser recolecta `AstLet { name, value }` en `AstProgram::lets`.
- El resolvedor construye `let_table: map<string, string>` con los textos de las expresiones.
- Resuelve cadenas iterativamente: `let A = B+1; let B = 2;` → tras iterar, `A = "2+1"`.
- En `toExpr()`, `evalExpr()` y `cloneSubstSdf()`, aplica `substExprText(text, let_table)` antes de procesar.
- `substExprText` respeta límites de palabra — no reemplaza `RADIO` dentro de `RADIOLOGIA`.

**Protección de built-ins:** Las variables predefinidas del evaluador (`pi`, `e`, `w`, `t`, `x`, `y`, `z`) tienen prioridad. Si alguien escribe `let pi = 3;`, el evaluador ignora el `let` porque los built-ins se checkean antes en `evalExpression()`.

**Diferencias clave con `#define`:**
| Aspecto | `let` | `#define` (C) |
|---------|-------|---------------|
| Límites de palabra | `RADIO` no afecta `RADIOLOGIA` | `RADIO` afectaría `RADIOLOGIA` |
| Resolución de cadenas | Iterativa, soporta dependencias | Una sola pasada |
| Ámbito | Global al archivo (sin bloques aún) | Global al archivo |
| Built-ins protegidos | Sí (`pi`, `e`, `w`, etc.) | No |

**Impacto ontológico:**
- `let` refuerza la naturaleza declarativa del lenguaje. No hay asignación en runtime, no hay estado mutable. La constante se expande y desaparece.
- En escenas densas (miles de nodos, cada uno con 10 referencias a `let`), el ahorro es `O(10 · n)` lookups evitados. El compilador absorbe ese costo una vez.
- Es una capa de abstracción que no filtra al RIH — el formato intermedio nunca se entera de que existió un `let`.

**Archivos modificados** (4): lexer, ast, parser, resolver. Cero cambios en RIH, renders, serialización.

## 9. Decisión: Múltiples luces (directional / point)

**Problema original:** El renderizador usaba una sola dirección de luz fija `light_dir = normalize(0.3, 0.8, 0.5)`. No había manera de definir luces personalizadas, luces puntuales con atenuación, o escenas con múltiples fuentes de luz.

**Solución:** Añadir un nuevo tipo de nodo al lenguaje: `light`/`lumbre`, que se resuelve a una tabla plana de `Light` en el RIH. El renderer itera sobre todas las luces acumulando contribuciones.

**Diseño de `Light`:**
- `LightType`: `DIRECTIONAL` (0) o `POINT` (1)
- `Light::direction`: vector normalizado para direccionales
- `Light::position`: origen para puntuales
- `Light::color`: color base de la luz
- `Light::intensity`: multiplicador
- `Light::falloff`: coeficiente de atenuación $1 / (1 + f \cdot d^2)$ solo para puntuales

**Impacto arquitectónico:**
- **AST**: nuevo `AstLight` con campos parseables. Lista `lights` en `AstProgram`.
- **Lexer**: 6 nuevos pares de keywords bilingües (`light`/`lumbre`, `type`/`tipo`, etc.).
- **Parser**: `parse_light()` y dispatch en `parse()`.
- **Resolver**: `resolveLight()` — convierte strings a `LightType`, evalúa vectores y escalares.
- **RIH**: `Light` struct, `LightRecord` binario, `num_lights` en header.
- **Serialización**: JSON escribe la tabla de luces; binario escribe records con string table para nombres.
- **Renderers**: se reemplazó la única `light_dir` por un `shadeWithLights()` que itera sobre la tabla de luces. Sombra ray solo para luces direccionales. Para puntuales, se calcula dirección y atenuación desde la posición del hit.
- **Backward compatibility**: si no hay luces en el RIH, se inyecta una luz direccional por defecto.

**Trade-offs:**
- El shading se vuelve $O(n \cdot m)$ donde $m$ es el número de luces. En escenas típicas con 1-4 luces el costo es marginal (~5% del tiempo total de render).
- Las sombras solo se calculan para luces direccionales (un shadow ray por luz). Para puntuales se omite por ahora — se añadirá sombreado puntual en una iteración futura.
- No hay herencia ni jerarquía de luces: la tabla es plana, como los materiales. Esto mantiene la simplicidad del modelo ontológico.

## 10. Estado actual del prototipo (resumen ejecutivo)

**Pipeline re‑verificado:**
- El lenguaje frontend (Éter) define el universo mediante entidades, materiales y reglas.
- El Resolvedor aplica las leyes ontológicas y genera un archivo intermedio (RIH) o directamente slots GTE.
- GTE (backend) produce `.ptr`, que PTR convierte a `.ctp`, CTB a `.tb`, y render a `.png`.
- Para el caso de prueba (onda expansiva), la expresión analítica se inyecta en un slot GTE sin necesidad de pre‑renderizado.

**Pipeline directo (operador_tb):**
- `.herm → [herm] → .rih → [operador_tb] → .tb → [render.exe] → .png`
- Renderizado por sphere tracing offline, sin dependencia del pipeline MLIR.

**Estadísticas del prototipo:**
- Resolución 800×600 → 7,500 bloques de 8×8.
- Compresión típica: ~1,900 bloques P0, ~1,500 PAL_RGBA, ~100 RAW.
- 40+ materiales en librería estándar.
- 12 primitivas SDF, 10 operadores SDF.

**Hitos completados (v0.9–v0.12):**
1. **Volumen rendering**: integración Beer-Lambert en ambos renderers (herm --render y operador_tb). Niebla, emisión volumétrica, blend con sólido.
2. **Defs parametrizadas**: `def "name"(params) { ... }` + `instance "name"(args)`. Sustitución textual en AST.
3. **Expresiones en SDFs**: `SdfNode::params` cambiado de `f32[4]` a `Expr[4]`. Ahora `sphere(1.5 * 0.5)`, `cylinder(h * 0.5, r)`, `sphere(0.65 + sin(w * 2) * 0.35)` son válidos. Evaluación por fotograma en `evaluateFrame()`.
4. **Documentación completa**: 11 documentos en `docs/` cubriendo todo el sistema.
5. **Múltiples luces**: `light`/`lumbre` nodos con tipo direccional o puntual, tabla de luces en RIH, acumulación en renderers. Sombra ray solo para direccionales.
6. **Variables `let`**: expansión textual en compilación vía `substExprText()`. Cadenas entre lets, compatible con defs parametrizadas. Built-ins (`pi`, `e`, `w`, etc.) protegidos.

## 11. Deuda técnica: PBR y BVH no son features del lenguaje, son librerías

**Problema identificado (v0.13, discusión con el creador):**
Los hitos planeados (PBR, BVH) estaban en el orden incorrecto y concebidos como implementaciones en C++ (renderer patches), no como extensiones del lenguaje.

**Simplificación:** No necesitamos un formato `.ether` nuevo. El `import` + `def` ya existe. Solo falta:
- `x, y, z` en el evaluador (ligar el punto del rayo)
- Que el renderer evalúe expresiones como SDFs (en vez de `switch (sdf_type)`)
- `hash()` determinista para imperfección natural

**Auditoría de sobreingeniería (v0.13):** Se identificaron múltiples casos de complejidad innecesaria:
- Pipeline MLIR/GTE/PTR/CTB descrito como parte del core cuando **es un proyecto aparte** (`Lienzo y renderizado/`). Docs corregidos.
- Formato `.rib` binario con escritor de 167 líneas pero **sin lector** — huérfano
- Flag `--tb` parseado pero **nunca ejecutado**
- 3 headers de contrato (`material.h`, `sdf.h`, `scene.h`) con **0% implementación**
- Dos renderers ~95% idénticos (~1500 líneas duplicadas)
- Bilingüe duplica ~340 líneas de lexer (tabla bidireccional bastaría)
- 14 librerías planeadas en `contrato-ontologico.md` — solo 1 existe

**Decisión:** Simplificar el roadmap al mínimo necesario:
1. `x, y, z` en el evaluador (~10 líneas)
2. Expresión SDF como texto en RIH (reemplazar `u8 sdf_type` + `f32 params[4]`)
3. Renderer evalúa expresión con `p=(x,y,z)` en cada paso
4. Mover primitivas a `std_sdf.hm`
5. `hash()` en evaluador
6. Librerías `.hm` de dominio (std_optics, std_geology, std_camera, std_biology)

**Archivo:** `docs/10-deuda-tecnica.md` y `docs/contrato-ontologico.md` actualizados.

---

## 12. Decisión: Abandono de la Física Estática (Plano Tensorial 1x8)

**Problema identificado (v0.14):**
Incluso tras la refactorización v0.13, el compilador y los motores de render (`herm_render.cpp` y `operador_tb/renderer.cpp`) seguían utilizando un modelo físico codificado: el PBR (Physically Based Rendering) estático con propiedades como `base_color`, `roughness`, `metallic`, e `emission`. Esto impedía a las bibliotecas `.hm` definir e inyectar nuevas heurísticas ópticas, pues el motor forzaba una evaluación predefinida de luz.

**Solución:** Reemplazar todas las propiedades codificadas por una abstracción fundamental de 8 dimensiones en la estructura del Material: el **Plano Tensorial 1x8** `[x, y, z, w, r, g, b, a]`.

**Impacto Arquitectónico:**
- **Evaluación unificada:** El motor ahora extrae los valores ópticos (`r, g, b, a` en los índices 4..7) de manera interactiva mediante las coordenadas de impacto `(x, y, z, w)` en tiempo de render.
- **Sintaxis Hermética:** El script `.herm` expone `tensor: [0, 0, 0, 0, 0.8, 0.3, 0.1, 1.0];` lo que elimina las descripciones de modelado 3D tradicionales en favor del determinismo algebraico puro.
- **Desbloqueo de librerías:** La simulación PBR (GGX, Fresnel) ya no es un motor blindado; a futuro las funciones residirán en bibliotecas puras (ej. `std_optics.hm`) interactuando en las coordenadas del tensor.

---

## Conclusión del historial

El lenguaje hermético ha evolucionado desde una idea filosófica ("jugar a ser Dios con matrices 1×8") hasta una arquitectura concreta de compilación, basada en SDFs exactas, tabla global de materiales, pirámide Física‑Química‑Biología, y un contrato sólido con el backend de renderizado. Las iteraciones y críticas han permitido descartar aproximaciones estadísticas (neural SDF), discretizaciones toscas (vóxeles) y soluciones que rompen la continuidad (polinomios lineales por celdas). El prototipo actual demuestra que el modelo es viable y que la separación entre el mundo ontológico y la capa de observación es la clave para la escalabilidad y el determinismo.
