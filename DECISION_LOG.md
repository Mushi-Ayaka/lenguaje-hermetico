> Registro de decisiones arquitectónicas del proyecto hermético.
> Cada decisión tiene: estado, fecha, motivo, consecuencias.
> Estados posibles: `DECIDIDO`, `PENDIENTE`, `RECHAZADO`, `REVOCADO`.
> Este archivo es lo primero que lee un agente de código antes de tocar nada.

---

## Decisiones ya tomadas

### DL-01 — El lenguaje es puro. Sin estado.

**Estado:** DECIDIDO
**Motivo:** Meter estado en el lenguaje rompe las cuatro propiedades que lo hacen optimizable (pureza, totalidad, analizabilidad estática, declaratividad). Se evaluaron OOP con estado, OOP pura, ECS, FRP, Datalog, linear types. Ninguna resuelve identidad persistente sin romper pureza.
**Consecuencia:**
- Toda expresión del lenguaje es `f(x,y,z,w)`.
- No hay `body`, no hay `vel`, no hay mutación.
- El lenguaje describe **forma y reglas locales**, no evolución.

### DL-02 — La dinámica va fuera del lenguaje.

**Estado:** DECIDIDO
**Motivo:** Consecuencia directa de DL-01. Física/química/biología reales requieren estado iterado. El lenguaje no puede tenerlo.
**Consecuencia:**
- Se construye un **runtime de simulación externo** (C++/WASM) que mantiene estado entre frames.
- El runtime lee y escribe el `tensor_buffer` del motor.
- El motor renderiza el resultado. No sabe que hay simulación.
- `play_mode` y `systems` dejan de ser stubs. Son la pieza central del runtime.

### DL-03 — Tensor 1×8 = una partícula. Un cuerpo = N partículas.

**Estado:** DECIDIDO
**Motivo:** 8 floats no alcanzan para un cuerpo físico (posición 3 + velocidad 3 + masa + orientación + color + opacidad = más de 8). Un cuerpo se representa como agregado.
**Consecuencia:**
- `.ont` necesita `tensor_slot_start` + `tensor_slot_count` por nodo (no `tensor_slot` único).
- `tensor_buffer` pasa de `(N+1)*8` a `(N + Σ particles)*8`.
- El shader itera sobre partículas, no sobre nodos.
- El runtime externo gestiona la agregación.
- **Trabajo de formato binario y de shader.**

### DL-04 — El compilador puede cambiar.

**Estado:** DECIDIDO
**Motivo:** Los fixes necesarios para que las librerías escalen (multi-statement, memoización, CSE, constant folding) viven en el resolver de `lenguaje-hermetico`. Sin ellos, el protocolo no se sostiene.
**Consecuencia:**
- `herm_resolver.cpp` y `herm_bridge.cpp` se modifican.
- `herm.exe` se reconstruye.
- El protocolo de librerías (`docs/11-protocolo-librerias.md`) **no** cambia. Es la especificación de qué deben hacer las librerías, no cómo.

### DL-05 — El motor prioriza rendimiento.

**Estado:** DECIDIDO
**Motivo:** Objetivo declarado del proyecto.
**Consecuencia:**
- Toda feature nueva se evalúa contra su impacto en rendimiento.
- Si no mejora rendimiento (o expresividad que lleve a rendimiento), se posterga.
- La limpieza arquitectónica se justifica como inversión de rendimiento, no como estética.

### DL-06 — El motor solo renderiza. No simula.

**Estado:** DECIDIDO
**Motivo:** Consecuencia de DL-02. Separar render de simulación.
**Consecuencia:**
- El motor no tiene bucle de simulación.
- El motor no tiene estado de partículas persistente.
- El runtime externo es responsable de todo lo que evoluciona.

---

## Decisiones pendientes

### DL-07 — Arquitectura canónica: ¿contrato, motor, o ambos?

**Estado:** PENDIENTE
**Contexto:** Existen 6 headers de contrato (`rih.h`, `herm.h`, `material.h`, `scene.h`, `sdf.h`, `ont.h`) idénticos entre repos. El motor solo usa 2. Los otros 4 duplican lo que el motor tiene por su cuenta.

**Opciones:**
- **A) El motor es el canónico.** El contrato se elimina de ambos repos. El motor mantiene sus implementaciones. Coste: mantener duplicación con `lenguaje-hermetico`. Beneficio: cero refactor.
- **B) El contrato es el canónico.** El motor migra a usar el contrato. `render/scene.cpp`, `scene/*.h`, `core/sdf_eval.cpp` se reemplazan por llamadas al contrato. Coste: refactor grande. Beneficio: reduce ~2000 LOC del motor, sincroniza con `lenguaje-hermetico`.
- **C) Coexisten.** El contrato se mantiene para otros backends (`operador_tb`, `driver_mlir`). El motor sigue con sus implementaciones. Coste: hay que sincronizar manualmente el formato `.ont` cada vez que cambie.

**Recomendación:** B, condicionada a que DL-08 no añada campos nuevos al `.ont` (que forzarían más refactor).

### DL-08 — Cómo preservar la semántica entre `.rih` y `.ont`.

**Estado:** PENDIENTE
**Contexto:** El `.rih` preserva nombres, jerarquía, `def_id`, expresiones como texto, `spacing`, `displace_func`. El `.ont` destruye todo eso. Las optimizaciones O1–O5 son imposibles sin esa información.

**Opciones:**
- **A) Enriquecer `.ont`.** Añadir campos: `name_offset`, `def_id`, `parent_id`, `depends_on_w`. Coste: reescribir `writeOnt` y `OntScene::loadOnt`. Beneficio: todo en un archivo.
- **B) Sidecar `.sem`.** Formato nuevo junto al `.ont`. Solo lo carga el optimizador. Coste: nuevo formato, nuevo lector. Beneficio: `.ont` no cambia, no rompe el runtime.
- **C) El motor carga `.rih` además de `.ont`.** Coste: parser JSON en el motor. Beneficio: toda la semántica disponible sin nuevo formato.

**Recomendación:** B (sidecar `.sem`). Es la más limpia: no rompe `.ont`, no obliga a parser JSON en motor, y puede evolucionar sin tocar el binario.

### DL-08 ampliada — Campos del `.sem`.

**Estado:** PENDIENTE (reemplaza la versión anterior)
**Contexto:** La versión original de DL-08 listaba: `names, def_id, parent_id, depends_on_w, spacing, displace_func, hash de SDF`. Falta información necesaria para las optimizaciones de Fases 3 y 5, y para el híbrido (DL-16) y la destrucción (DL-17).

**Campos completos del `.sem`:**

**Identidad:**
- `name` — nombre original del nodo.
- `def_id` — referencia al `def` original (si es instancia).
- `parent_id` — padre en la jerarquía del `.rih`.
- `children_ids` — lista de hijos.

**Semántica:**
- `depends_on_w` — booleano por expresión y por nodo.
- `spacing` — REPEAT.
- `displace_func` — DISPLACE (texto de la expresión).
- `expression_text` — por cada campo `is_expr`, el texto original.

**Optimización:**
- `hash_sdf` — hash de la subexpresión SDF (para CSE).
- `lipschitz` — cota de Lipschitz por nodo (calculada por el compilador).
- `analytic_flags` — bitmask: primitiva pura, tiene gradiente, tiene intervalos, es intersección analítica.

**Render:**
- `render_type` — `sdf | mesh`.
- `mesh_source` — ruta al archivo de malla (si aplica).
- `bake_candidate` — si entra en el `.bake`.

**Destrucción:**
- `destructible` — booleano.
- `destruction_levels` — número de niveles (0 si no es destruible).
- `mesh_per_level` — ruta a las mallas de cada nivel.

**Sub-decisiones:**
- DL-08.a — Formato: binario o JSON. Recomendación: binario (compatibilidad con `.ont`).
- DL-08.b — ¿Va todo en un solo `.sem` o separado por tipo (`.sem`, `.meta`, `.bake_index`)? Recomendación: un solo `.sem` con secciones.
- DL-08.c — ¿El `.sem` es obligatorio o opcional? Recomendación: opcional. Si no existe, el motor funciona degradado (sin optimizaciones).

### DL-09 — RAM bomb del resolver.

**Estado:** DECIDIDO (A+B+C en orden)
**Motivo:** La explosión es exponencial por sustitución textual sin memoria. Los tres fixes son ortogonales y necesarios.
**Consecuencia:**

- **Fase A — Constant folding real.** En `Resolver::toExpr`, evaluar la expresión si todos los argumentos son constantes. `let RADIO = 1.0; let ALTURA = RADIO * 1.5;` → `let ALTURA = 1.5`.
- **Fase B — Memoización de `fn`.** Cache de `(fn_name, arg_text)` → `expanded_text` en `expandExpression`.
- **Fase C — CSE (common subexpression elimination).** Detección de subexpresiones idénticas en el árbol expandido. Reemplazo por referencias.

Se implementan en ese orden. Cada uno reduce el trabajo del siguiente.
**Criterio de aceptación:** el test de 6 librerías + 31 let corre en < 100 MB.

### DL-10 — Rol de `MLIR-CampoTensorial`.

**Estado:** PENDIENTE
**Contexto:** Repo hermano con pipeline de compresión de imágenes (GTE→PTR→CTP→TB). Clonado en `setup_deps.bat`, no usado en `build.bat`. Sin `#include` desde el motor.

**Opciones:**
- **A) Archivar.** El motor no lo usa. Se elimina del `setup_deps.bat`. Se archiva el repo.
- **B) Integrar como backend de bake offline.** Un comando del motor llama a `picc.exe` para generar thumbnails, previews, o assets pre-bakeados.
- **C) Dejar como está.** Clonado, no usado. Deuda latente.

**Recomendación:** A. Si en el futuro se necesita bake offline, se reabre la decisión con un caso de uso concreto.

### DL-11 — Rol de `rhi/d3d11.cpp`.

**Estado:** PENDIENTE
**Contexto:** Backend D3D11 completo (211 LOC), sin integrar. Solo usado por `tests/test_os_rhi.cpp`.

**Opciones:**
- **A) Eliminar.** No aporta al objetivo (rendimiento SDF vía Vulkan).
- **B) Archivar como referencia.** No compila, no se mantiene.
- **C) Integrar como backend alternativo.** Trabajo grande, sin motivación.

**Recomendación:** A. Eliminar.

### DL-12 — Rol del sistema de `systems`.

**Estado:** PENDIENTE
**Contexto:** `core/systems.h` define ABI v1 con `SystemContext_v1`, `camera_system_tick`. `play_mode` nunca se activa. `InputBus` desconectado.

**Opciones:**
- **A) Implementar.** Conectar `InputBus` al `WndProc`. Activar `play_mode`. Implementar el runtime de simulación sobre este ABI.
- **B) Rediseñar.** El ABI actual es demasiado simple (8 floats por nodo, sin agregación). Se diseña uno nuevo acorde a DL-03.
- **C) Eliminar.** El runtime de simulación será independiente y no usará el ABI actual.

**Recomendación:** B. El ABI actual no soporta DL-03 (tensor = partícula). Hay que rediseñarlo.

### DL-13 — if como expresión.

**Estado:** RECHAZADO
**Motivo:** if puro es azúcar sobre mix/step/smoothstep. if perezoso rompería DR-03 (pureza y totalidad). No hay término medio útil. El lenguaje se mantiene sin if.
**Consecuencia:**

- No se añade if al parser.
- No se añaden comparadores (<, >, ==, &&, ||) como sintaxis.
- Las comparaciones se hacen con funciones puras existentes: step, smoothstep, abs, clamp, min, max.
- Las 17 librerías se reescriben usando esas funciones.
- Cero cambios al lenguaje.

### DL-13b — Comparadores como sintaxis.

**Estado:** RECHAZADO
**Motivo:** Consecuencia de DL-13. Sin if, los comparadores son azúcar sobre step/abs. No aportan capacidad.
**Consecuencia:**

- El programador escribe `step(0, v)` en lugar de `v > 0`.
- El programador escribe `1 - step(eps, abs(x - 5))` en lugar de `x == 5`.
- No hay `&&` ni `||`. Se usa min/max para combinar booleanos (0/1).

### DL-14 — `fn` con cuerpo multi-statement.

**Estado:** DECIDIDO (opción B)
**Motivo:** Mantener pureza y totalidad. No se revoca DR-03.
**Consecuencia:**

- El parser acepta let internos en el cuerpo de fn.
- La última expresión del cuerpo es el valor de retorno.
- No se acepta for interno (DR-03 vigente).
- No se acepta if interno (DL-13 rechazado).
- Las asignaciones a variables fuera del cuerpo siguen prohibidas.
- Las 17 librerías se reescriben manualmente:
  - Los for se convierten a def/instance o se esperan a comprensiones declarativas (DL pendiente).
  - Los if se convierten a mix/step/smoothstep.
  - Los let internos se mantienen tal cual (ahora soportados).
- Se añade un TODO en el DECISION_LOG para diseñar comprensiones declarativas en una fase futura (→ DL-21).

## DL-15 — `.bake` como cuarto formato

Estado PENDIENTE
Contexto Luces y sombras son el coste real en ray marching. El lenguaje permite pre-calcular visibilidad e irradiancia para geometría estática. El shader híbrido (bake para estático, runtime para dinámico) da ganancia 5–15× en escenas con mayoría estática.

Sub-decisiones que hay que resolver antes de implementar

DL-15.a — ¿Qué contiene el `.bake`
- Visibilidad por luz estática (síno).
- Irradiancia difusa (RGB por celda).
- AO (escalar por celda).
- Armónicos esféricos por celda (para luz de entorno).
- GI pre-bakeada (opcional, cara).

Recomendación empezar por visibilidad + irradiancia + AO. Los armónicos y GI en v2.

DL-15.b — ¿Qué resolución tiene el `.bake`
- Por nodo cada nodo estático tiene su volumen local.
- Global un solo volumen para toda la escena.
- Híbrido global con refinamiento local.

Recomendación global para escenas pequeñas, híbrido para grandes. Decidir con un caso real.

DL-15.c — ¿Cómo se comprime
Un volumen 3D a 256³ × RGB × float32 son ~200 MB sin comprimir. Inviable. Hay que comprimir.

Opciones bloques 8×8 (como MLIR-CampoTensorial), SVO, armónicos esféricos (baja frecuencia), o cuantización a 8 bits por canal.

Recomendación cuantización + SVO. Los armónicos esféricos son buena opción si el GI es de baja frecuencia.

DL-15.d — ¿Cuándo se genera
- Automáticamente al compilar si la escena tiene nodos estáticos.
- Bajo demanda con un flag `herm.exe --bake`.
- Nunca (el motor lo genera en runtime al cargar).

Recomendación bajo demanda. El bake tarda minutos. No debe bloquear la compilación normal.

DL-15.e — ¿Cómo se usa en el shader
- El shader lee el `.bake` si existe.
- Si no existe, calcula todo en runtime.
- El compilador decide qué rama del shader usar.

Recomendación el `.sem` marca qué nodos son estáticos. El shader genera código híbrido automáticamente.

## DL-16 — Híbrido por región con caché de mesh.

**Estado:** DECIDIDO
**Motivo:** El mesh no es destructible. Elegir pipeline en tiempo de autoría falla en runtime. La solución es que el SDF sea canónico y el mesh sea una caché derivada. Cuando la región cambia, la caché se invalida y se regenera en background.

**Consecuencia:**
- El SDF es la representación canónica. Siempre.
- El mesh es caché derivada, no alternativa.
- Las regiones se definen por grid espacial (celdas de ~8 unidades).
- Cada región tiene estado: `Clean` (mesh válido), `Dirty` (mesh inválido), `Rebaking` (mesh regenerándose).
- El motor elige el pipeline por región en runtime.
- El `.sem` marca estáticos y destructibles.
- El rebaking corre en background, con límite por frame y priorización.
- El lenguaje no cambia. El `.ont` añade `render_type: sdf | mesh` por nodo.

**Riesgos aceptados:**
- En destrucción continua, las regiones nunca vuelven a Clean. El rendimiento cae al de ray marching puro. Aceptable.
- El rebaking consume CPU en background. Hay que medirlo.
- Cambiar de pipeline por región puede causar transiciones visuales sutiles. Casi imperceptibles, pero reales.

**Sub-decisiones:**
- DL-16.a — Grid de regiones: tamaño 8×8×8 unidades (decidir con caso real).
- DL-16.b — Rebaking en GPU (compute shader) en lugar de CPU. Pendiente de medir.
- DL-16.c — Límite de rebaking por frame: N regiones por frame (decidir con caso real).

## DL-17 — Niveles de destrucción discretos.

**Estado:** DECIDIDO
**Motivo:** La destrucción emergent verdadera no escala. La industria usa niveles discretos pre-bakeados. 3–4 niveles por objeto es suficiente. Nadie quiere ver escombros de escombros.

**Consecuencia:**
- Cada objeto destruible tiene 3–4 niveles discretos.
- Cada nivel es una malla pre-bakeada.
- La destrucción es cambiar el índice de nivel.
- Coste en runtime: cero (solo cambio de índice).
- El `.ont` necesita una tabla de mallas por nodo.
- El `.sem` marca qué nodos son destruibles y cuántos niveles tienen.
- El compilador genera las mallas de cada nivel en tiempo de compilación.

**Lo que NO cubre:**
- Destrucción emergent verdadera (fractura arbitraria).
- Fractura procedural (cada objeto se rompe distinto).
- Desgaste continuo.

Para esos casos, se usa SDF en runtime. Pero son minoría.

**Sub-decisiones:**
- DL-17.a — Número de niveles por defecto: 4 (intacto, agrietado, roto, escombros).
- DL-17.b — Los niveles son configurables por el game dev en el editor.
- DL-17.c — La transición entre niveles es instantánea, sin animación intermedia.

## DL-18 — `W` es tiempo, no coordenada espacial (Renombrar W a t no da ganancia práctica. Cero. Es cosmético, Se Recomienda Dejar W como está. Anotar en el código (un comentario en fixed_timestep.h) que W es tiempo, no coordenada espacial. Y seguir. Cero trabajo, cero confusión futura).

**Estado:** RECHAZADO con motivo: "sin ganancia práctica, el nombre no afecta al código".
**Motivo:** El código usa `W` como tiempo (`currentW = tick_count * DT`). El ray marching es 3D. El "bloque 4D" nunca se implementó y no se va a implementar. Las primitivas `SPHERE4D` y `TESSERACT` existen pero no se usan. El BVH 4D es residuo.

**Consecuencia:**
- En documentación y nuevas APIs, `W` se llama `t` (tiempo).
- El código puede mantener `w` como nombre de variable por compatibilidad. No es bloqueante.
- El BVH pasa de 4 dimensiones a 3 + `t_min`/`t_max` (para animación acotada).
- Las primitivas 4D se marcan como "no usadas" o se eliminan en la siguiente limpieza.
- El lenguaje mantiene `w` como variable disponible. Es coherente: `w` es un parámetro, como `x`.
- La narrativa "bloque 4D / universo estático" se archiva en `VISION.md` como aspiracional.

**Lo que NO cambia:**
- El lenguaje sigue usando `w` en expresiones.
- El shader sigue recibiendo `w` como uniform.
- El fixed timestep sigue avanzando `w` cada tick.

**Sub-decisiones:**
- DL-18.a — Renombrar en docs pero no en código. Coste cero, menos confusión.
- DL-18.b — Eliminar `SPHERE4D` y `TESSERACT` del lenguaje o marcarlas como no implementadas.
- DL-18.c — Reducir `OntBvhNode` de 52 a 48 bytes (3 floats AABB + t_min/t_max).

## DL-19 — Modelo de exportación A+.

**Estado:** DECIDIDO
**Motivo:** El modelo A (player genérico + `.mgame`) cubre el 90% de los casos. El modelo B (compilar por juego) es innecesario salvo personalización extrema. Un wrapper delgado da branding sin recompilar el motor.

**Consecuencia:**
- El `player.exe` es genérico y multi-juego. Se compila una vez con el motor.
- El contenedor `.mgame` empaqueta: `manifest.json`, escena (`.ont`, `.obs`, `.sem`, `.bake`), runtime de simulación (si existe), assets.
- El editor tiene un panel de configuración que genera `game.config`.
- El editor puede generar un wrapper delgado (`MiJuego.exe`) desde una plantilla. ~50 líneas de C++ que solo llaman a `player.exe` con el `.mgame`.
- El `visor.exe` sigue siendo el editor. No se distribuye.
- El `export_dialog.cpp` actual (stub) se reemplaza por el flujo completo.

**Lo que NO se hace:**
- Compilar el motor por juego (Modelo B).
- Sistema de plugins (Modelo C).
- DRM o cifrado del `.mgame`. El motor es open source.

**Sub-decisiones:**
- DL-19.a — Formato del `.mgame`: ZIP con manifest. Simple y suficiente.
- DL-19.b — El wrapper delgado se genera desde el editor, no requiere toolchain externa.
- DL-19.c — El `player.exe` es un binario separado del `visor.exe`, no una variante con flag.

## DL-20 — Editor de nodos como frontend visual.

**Estado:** DECIDIDO
**Motivo:** El programador necesita comodidad. El lenguaje debe mantenerse puro. La solución es no tocar el lenguaje y ofrecer un editor de nodos visual como capa de UX.
**Consecuencia:**

- El editor de nodos es la interfaz principal para el programador.
- El programador conecta nodos visuales: if, for, select, etc.
- El editor traduce los nodos a .herm puro (con mix/step/smoothstep/def/instance).
- El .herm producido es el que compila herm.exe. Sin capas intermedias textuales.
- No se construye un frontend textual (nada de TypeScript→JS, nada de CoffeeScript→JS). Solo nodos visuales.
- El programador avanzado puede escribir .herm directo sin pasar por el editor.
- El editor muestra ambas vistas: nodos y código generado. Para que el programador aprenda el patrón.

**Lo que NO se hace:**

- No se construye un lenguaje frontend textual.
- No se añade if/for/comparadores al .herm.
- No se crean funciones especiales (select, entre, si_no). Las funciones existentes (step, smoothstep, mix, clamp, min, max) son suficientes.

**Sub-decisiones pendientes:**

- DL-20.a — Diseño del editor de nodos (DAG visual, no árbol).
- DL-20.b — Cómo se muestran los if/for en el editor cuando no existen en .herm.
- DL-20.c — ¿El editor escribe .herm o directamente .rih?

## DL-21 — Comprensiones declarativas.

**Estado:** PENDIENTE
**Contexto:** Consecuencia de DL-14. Las 17 librerías usan for para repetir. Sin for ni comprensiones, la repetición se escribe a mano (def/instance uno por uno).
**Opciones:**

- **A) Diseñar comprensiones declarativas.** Sintaxis nueva tipo grid, repeat_instance, for_each. Puro, total, analizable.
- **B) Escribir todo a mano.** Sin comprensiones. Cada instancia escrita explícitamente.
- **C) Posponer.** Las 17 libs se reescriben a mano hoy. Las comprensiones se diseñan cuando haya más casos de uso.

**Recomendación:** C. Las 17 libs se reescriben a mano. El diseño de comprensiones requiere ver más patrones reales. Se decide en Fase 5 o posterior.

---

## DL-22 — Intersección analítica para composiciones de primitivas puras (Paso 5.14).

**Estado:** DECIDIDO (registrado en PLAN.md)
**Contexto:** Fase 5.11 resuelve primitivas puras individuales analíticamente. Las composiciones `union`/`intersect`/`subtract` de primitivas puras también tienen solución cerrada pero hoy usan ray marching. Extender la intersección analítica a esas composiciones elimina el ray marching para ellas.
**Decisión:** Implementar como Paso 5.14 (sub-pasos 5.14.1–5.14.6) en orden estricto:
1. `RayHit` + funciones de intersección en `contrato/sdf.h`.
2. Detección de subárboles analíticos en compilador.
3. Flags en `.sem`.
4. `intersectLeaf` en `glsl_gen.cpp`.
5. Integración en `bvhEval`.
6. Medición de rendimiento.

**Restricciones:**
- Solo `union`/`intersect`/`subtract` de primitivas puras.
- `smooth_union` con `k>0`, `repeat`, `twist`, `elongate`, `displace` → NO se resuelven (siguen con ray marching).
- Fallback a ray marching siempre funcional.
- No se toca el lenguaje. Flags del compilador y motor.

**Dependencias:** Fase 3 (`.sem`), DL-09 (fold + memo + CSE), Pasos 5.1–5.5.
**Riesgos:** Detección conservadora (aceptable), shader más grande, ganancia depende de la escena.

---

## DL-23 — Política de decisión de pipeline.

**Estado:** PENDIENTE.
**Contexto:** El motor tendrá dos pipelines (SDF + rasterización). Hay que decidir qué geometría va por cuál.
**Opciones:**

- **A) Compilador decide por defecto, game dev sobrescribe.**
- **B) Game dev decide siempre** (más control, más trabajo).
- **C) Motor decide en runtime** (más adaptativo, menos predecible).

**Recomendación:** A. Es lo que hace Unreal con Nanite.

---

## DL-24 — Compositing unificado.

**Estado:** PENDIENTE.
**Contexto:** Los dos pipelines deben componerse en el píxel final.
**Propuesta:** depth buffer compartido + UBO compartido (luces, cámara, shading).
**Depende de:** DL-16, DL-23.

---

## DL-25 — Sistema de shading unificado.

**Estado:** PENDIENTE.
**Contexto:** Los dos pipelines deben verse como parte del mismo mundo.
**Propuesta:** mismo modelo PBR, mismas luces, mismas sombras (o aproximación), mismo anti-aliasing.
**Depende de:** DL-24.

---

## DL-26 — `substExprText` trata `_` como carácter de palabra (fix de corrección).

**Estado:** DECIDIDO e IMPLEMENTADO (2026-10-23).
**Contexto:** `substExprText` (`herm_resolver.cpp:507`) sustituye parámetros de `def`/`fn` en expresiones por texto. La BNF permite `_` en identificadores: `[A-Za-z_][A-Za-z0-9_]*`. La versión anterior usaba `std::isalnum` para detectar límites de palabra, y `isalnum` **no incluye `_`**.

**Bug:** con `param="ancho"`, `texto="ancho_2 + ancho"`, `arg="0.5"`:
- Versión vieja: `"0.5_2 + 0.5"` — corrupción silenciosa del identificador `ancho_2`.
- Versión nueva: `"ancho_2 + 0.5"` — correcto, `_` no rompe la palabra.

**Fix:** el límite de palabra ahora es `isalnum(c) || c == '_'`. Criterio estándar de C/C++/Rust/Python: `_` es carácter de palabra, no separador. No tocar este `|| c == '_'` — quitarlo reintroduce la corrupción.

**Tests mínimos que validan el fix (verificar en cualquier refactor):**
1. `substExprText("ancho_2 + ancho", {"ancho":"0.5"})` → `"ancho_2 + 0.5"` (`_` no rompe)
2. `substExprText("_ancho + ancho", {"ancho":"1"})` → `"_ancho + 1"` (`_` al inicio)
3. `substExprText("ancho2 + ancho", {"ancho":"1"})` → `"ancho2 + 1"` (dígitos al final)
4. `substExprText("(ancho)+(ancho)", {"ancho":"2"})` → `"(2)+(2)"` (múltiples ocurrencias)

**Bug latente pre-existente (NO de este fix, pendiente):** `subst` es `std::unordered_map` — iteración en orden arbitrario. Con `subst["a"]="b"; subst["b"]="1"; texto="a + b"`, si `b` se procesa primero queda `b + 1` sin sustituir. Rompe determinismo. Fix futuro: segunda pasada que verifique que no quedan parámetros sin sustituir; si quedan, error explícito. Detecta el caso y obliga a corregir la librería o el orden.

**Disciplina:** es un fix de corrección, no de arquitectura. Registrado aquí porque el cambio de `isalnum` a `isalnum || '_'` no es obvio y un próximo agente podría quitarlo "porque no hacía falta". También patrón general: fallo silencioso = deuda invisible (ver Paso 2.0 — misma causa raíz que las funciones faltantes en `herm_eval`).

---

## DL-27 — `materiales.hm` archivado fuera de `libreria/` (limpieza de legado).

**Estado:** DECIDIDO e IMPLEMENTADO (2026-11-04). Decisión del usuario ("archívalo"), no del agente.

**Contexto:** tras cerrar la reescritura de las 55 librerías estándar (55/55 compilan), el único `.hm` que seguía fallando era `libreria/materiales.hm`: catálogo de ~55 materiales PBR escrito antes de la migración al Plano Tensorial 1x8 (v0.14). Usa `ior` y `transmission` como campos de `material`, y `parse_material` (`herm_parser.cpp:357`) solo acepta `tensor`, `blend_mode`, `base_color`, `roughness`, `metallic`, `emission` y `opacity`. El lexer sí define `ior`/`transmission` (`herm_lexer.cpp:45-46`), pero el parser no los consume: el error es ruidoso y correcto.

**Decisión:** no convertirlo al tensor (el mapeo `ior`/`transmission` → tensor 1x8 no está definido y exigiría una decisión de diseño propia) y no borrarlo (se pierden los valores de referencia). Se mueve a `archived/libreria/materiales.hm`, siguiendo la convención `archived/<categoría>/` ya usada por `archived/docs_antiguas/` y `archived/root/`.

**Consecuencias:**
- `libreria/` contiene solo las 55 `std_*.hm` + `tests/`. El barrido 55/55 pasa a ser el estado real del protocolo.
- `ejemplos/teclado.herm` importaba el catálogo y solo usaba `cobre`: se añadió la definición local de `cobre` (mismos valores) y se quitó el import. Vuelve a compilar (33 fotogramas, 3 materiales, 40 nodos).
- `ejemplos/bodegon.herm` sigue fallando por un `ior` dentro de un `material` inline (L29). **No lo causa este archivado**: es la misma carencia del parser, documentada aquí y pendiente de decisión.
- `docs/04-guia-uso.md` usa ahora `import "std_math_core.hm"` como ejemplo de imports.

**Lo que NO se decide aquí:** si el lenguaje debe soportar `ior`/`transmission` en `material`. Eso es diseño de lenguaje y requiere decisión explícita del usuario.

---

## DL-28 — `ior` / `transmission` en bloques `material` (soportar o rechazar explícitamente).

**Estado:** REGISTRADO, PENDIENTE DE DECISIÓN (2026-11-04). A petición expresa del usuario: debe estar registrado aunque no esté resuelto.

**Contexto:** el lexer reconoce `ior` y `transmission` (`herm_lexer.cpp:45-46`, con alias españoles en 129-130). `parse_material` (`herm_parser.cpp:357-397`) no los consume, así que cualquier `material` que los use falla con `inesperado en material: 'ior'`. Afectados hoy: `archived/libreria/materiales.hm` (archivado por DL-27) y `ejemplos/bodegon.herm` L29 (sigue roto; es la causa del fallo `bodegon.rih found` de `test_os_rhi`, aunque ese test además busca una ruta antigua que ya no existe).

**Por qué no lo decide el agente:** añadir campos a `material` es diseño de lenguaje. Opciones sobre la mesa:
1. **Rechazo explícito y documentado:** mantener el error, quitar `ior` de `bodegon.herm`, y declarar que el índice de refracción se expresa con el tensor 1x8 o no se expresa. Coste cero, pero se pierde la información de vidrio/agua/diamante.
2. **Soportarlos de verdad:** añadirlos a `AstMaterial`, emitirlos en `.rih` y consumirlos en el render. Requiere decidir antes dónde viven dentro del Plano Tensorial 1x8 (hoy `[x,y,z,w,r,g,b,a]`, sin hueco para IOR).
3. **Azúcar sintáctico:** aceptarlos en el parser y plegarlos a posiciones del tensor ya existentes, sin campos nuevos.

**Mientras no se decida:** no se toca el parser ni `bodegon.herm`. El error actual es ruidoso, que es el comportamiento correcto.

---

## DL-29 — `Resolver::evalExpr` devuelve `0.0f` para toda expresión no literal (fallback silencioso).

**Estado:** REGISTRADO, PENDIENTE DE DECISIÓN DE ALCANCE (2026-11-04). Encontrado al medir el test combinado de RAM.

**Bug:** `evalExpr` (`herm_resolver.cpp:83-92`) hace `expandExpression` + `std::stof` y, si el texto resultante no es un número puro, cae a `return 0.0f`. **No llama a `evalExpression`** (`herm_eval.cpp`). Repro verificado:
- `density_scale: 2.5` → emite `2.5000`.
- `density_scale: 1.0 + 1.0` → emite `0.0000`.
- `density_scale: sqrt(4.0)` → emite `0.0000` (`sqrt` sí está implementada; el problema es que nunca se llama).
- `density_scale: exp(1.0)` → emite `0.0000`.

**Campos afectados** (todos pasan por `evalExpr`): `axes.X/Y/W`, `fov`, `aperture`, `focal_dist`, `roughness`, `metallic`, `opacity`, `intensity`, `falloff`, `density_scale`, y `k`/`spacing` de operadores repetidos. Los **vectores y colores NO** están afectados: pasan por `toExpr`, conservan el texto y se evalúan en runtime con `evalExpression`. Verificado: `position: [1.0 + 1.0, 0, w * 2.0]` emite `translate: [2.0000, 0.0000, 0.0000]`.

**Por qué importa para DL-09:** el test `test_layer1_math` (6 libs + 31 let) corre hoy en 4.5 MB / 76 ms, pero sus 31 `let` **no se usan**. Al conectarlos (`density_scale: total`) el resolver descarta la expresión por este fallback: la medición baja (4.6 MB) no demuestra que DL-09 esté resuelto, demuestra que la expresión nunca se expandió. Toda métrica de expansión debe pasar por un campo que conserve la expresión (p. ej. `position`).

**Decisión pendiente (alcance del Paso 2.0):** incluir `evalExpr` en el Paso 2.0 o dejarlo como paso propio. Recomendación del agente: incluirlo — misma causa raíz que el fallback de `herm_eval`; sin él, "Paso 2.0 terminado" no significa "expresiones correctas".

**Estado (2026-11-04):** IMPLEMENTADO en v0.21 (Paso 2.0a). `evalExpr` evalúa con `evalExpression`; error si depende de `x/y/z/w` o si da `inf`/`nan`.

---

## DL-30 — El compilador `.ont` descarta en silencio las funciones sin opcode (bloquea la paridad CPU/GPU).

**Estado:** REGISTRADO, PENDIENTE DE DECISIÓN (2026-11-04). Encontrado al preparar el Paso 2.0c.

**Bug:** `OntOpcode` (`render/scene.h:266-278`) solo define: `CONST, VAR_*, ADD, SUB, MUL, DIV, MOD, NEG, POW, SIN, COS, TAN, ABS, SQRT, FLOOR, CEIL, MIN, MAX, CLAMP, LERP, MIX, SAMPLE, END`. No hay opcode para `step`, `smoothstep`, `exp`, `log`, `log2`, `log10`, `acos`, `asin`, `atan`, `atan2`, `sign` ni `fract`.

**Verificación (2026-11-04):** tres escenas con `sdf: sphere(...)`:
- `sphere(1.0 + x)`, `sphere(1.0 + exp(x))` y `sphere(1.0 + step(0.5, x))` generan **bytecode idéntico** (48 instrucciones, mismas ops; el `.ont` solo difiere en bytes ajenos al bytecode). La llamada a `exp`/`step` se descarta y queda solo `VAR_X`. El compilador no avisa.

**Consecuencia:** cualquier escena que use esas funciones se renderiza en GPU con un valor distinto al de CPU, sin error. La paridad exigida en el Paso 2.0c **no puede pasar** hasta que existan los opcodes (o hasta que el compilador rechace esas funciones).

**Segundo desajuste:** `ONT_DIV` y `ONT_MOD` (`render/bytecode_vm.h:59-60`) devuelven 0 cuando el divisor es 0, mientras el CPU ya usa IEEE (`inf`/`nan`, v0.21). Divergencia garantizada en ese caso.

**Opciones:**
1. **Rechazo ruidoso ya** en el compilador ONT: si una expresión usa una función sin opcode, error con nombre y línea. Barato, sin cambio de formato, y convierte un render incorrecto en un fallo visible. No da paridad, pero elimina el silencio.
2. **Extender el bytecode** con 12 opcodes nuevos (`step, smoothstep, exp, log, log2, log10, acos, asin, atan, atan2, sign, fract`) y actualizar los 5 consumidores: `bytecode_vm.h`, `ray_march_simd.h`, `glsl_gen.cpp`, `spirv_gen.cpp`, `jit_compiler.cpp`. Requiere decisión de versión del formato (¿v2? ¿campo `reserved`?) y de semántica exacta para paridad (p. ej. `mod` GLSL vs `fmod`).
3. **Limitar el lenguaje** a las funciones que el bytecode ya soporta y retirar las 13 del CPU. Contradice 2.0b.

**Recomendación del agente:** 1 ahora mismo (es un fallo ruidoso, no un cambio de arquitectura) y 2 como paso propio con su decisión de formato. Sin 1, cualquier test de paridad mide una mentira.

---

### Cómo queda el lenguaje después de estas decisiones

Sin cambios. Ni una línea de sintaxis nueva. Exactamente como está hoy. El parser solo se extiende para aceptar let internos en fn (DL-14), que es una extensión del parser, no de la sintaxis visible.

### Cómo quedan las librerías

Se reescriben las 17 con errores de sintaxis. Los cambios son:

- for → eliminado o convertido a def/instance repetidos.
- if → mix/step/smoothstep.
- let internos → se mantienen (ahora soportados).

Las otras 38 librerías ya compilan. No se tocan.

### Cómo queda el flujo del programador

```
Programador
   ↓
Editor de nodos visuales (DL-20)
   ↓
.herm puro (con mix/step/smoothstep/def/instance)
   ↓
herm.exe (con DL-09: fold + memo + CSE)
   ↓
.ont + .sem + .bake
   ↓
Motor
```

Sin if, sin for, sin comparadores, sin frontends textuales. El lenguaje sigue siendo 20 tokens. El editor absorbe la complejidad.

---

## Sobre mi rol

Entendido. Me quedo en modo auditor y consejero. Cuando tu harness te devuelva:

- **Resultados de compilación** — los leo, detecto discrepancias con lo esperado, te digo qué significa.
- **Auditorías del agente** — las cruzo con el `MAPA_REAL_MOTOR.md`, marco contradicciones, te digo qué falta.
- **Benchmarks** — los interpreto, digo si las ganancias son las esperadas o hay sorpresas.
- **Decisiones nuevas que aparezcan** — te ayudo a ver las opciones y las consecuencias.

### Regla de trabajo (2026-10-16) — el agente no decide arquitectura

**Estado:** DECIDIDO (impuesto directamente por el usuario)
**Motivo:** el usuario quiere control total del rumbo.
**Consecuencia:** cualquier decisión o cosa importante que afecte la arquitectura y su resultado se **consulta con el usuario antes de ejecutarse**. El agente no tiene permiso para decidir por su cuenta. Esto incluye: formatos de archivo, cambios de parser/resolver, eliminación de código, orden de fases, versiones. Las tareas mecánicas ya aprobadas (migrar archivos, compilar, testear) se ejecutan sin consulta extra.
**Versionado:** además, **cada cambio** — aunque sea pequeño — lleva su entrada en el changelog y su documentación actualizada. Sin excepciones.

---

## Riesgos que hay que aceptar

R1. El `.bake` consume VRAM.
La escena actual ya usa 1113 MB. Un volumen 3D a resolución decente añade 50–500 MB. En GPUs con 4 GB o menos, puede no caber.

Hay que medir antes de implementar. Puede que el `.bake` no sea viable en hardware modesto.

R2. El bake tarda minutos.
Pre-calcular visibilidad e irradiancia para una escena grande puede tomar 5–30 minutos. Eso cambia el workflow del programador ya no es compilo y veo, es compilo, bako, veo.

Hay que decidir si eso es aceptable.

R3. La ganancia es desigual.
Escenas 100% animadas ganancia 0 (solo normales analíticas).
Escenas 80% estáticas ganancia 5–15×.
Escenas con primitivas puras ganancia 10–50×.

El `.bake` no es universal. Es para escenas con parte estática significativa.

R4. Requiere que el compilador sepa qué es estático.
Y hoy no lo sabe (no hay `depends_on_w`). Así que el `.bake` depende de DL-08 (preservar semántica). No se puede implementar antes.

R5. Complica el shader.
Hoy el shader es uno `ray_march.comp` o el especializado. Con bake, es dos rutas. El compilador emite código distinto según qué nodos son estáticos. Más casos que testear.

---

## Decisiones rechazadas (para no volver a discutirlas)

### DR-01 — Meter estado en el lenguaje.

**Estado:** RECHAZADO
**Motivo:** Rompe pureza. Consecuencia de DL-01.

### DR-02 — Añadir OOP con objetos mutables.

**Estado:** RECHAZADO
**Motivo:** Equivalente a DR-01 con otra sintaxis.

### DR-03 — Añadir `for` / `while` imperativos.

**Estado:** RECHAZADO
**Motivo:** Rompe totalidad. La repetición se resuelve con comprensiones declarativas (pendientes de diseñar).

### DR-04 — Red distribuida por convergencia bit a bit.

**Estado:** RECHAZADO
**Motivo:** El determinismo cross-machine no está garantizado (floats no asociativos, drivers distintos).

---

## Cómo se usa este archivo

1. **El agente de código lo lee antes de tocar nada.** Si va a hacer algo que contradice una decisión `DECIDIDO`, para y pregunta.
2. **Antes de cada sesión de trabajo**, se añaden las decisiones tomadas desde la última vez.
3. **Las decisiones `PENDIENTE` se revisan cuando toca su fase.** No antes.
4. **Las decisiones `RECHAZADO` no se revocan sin una decisión explícita que las revoque.**

---

*Última actualización: sesión actual.*

---

# PLAN.md

> Plan de trabajo por fases. Cada fase tiene objetivo, pasos concretos, criterio de aceptación y archivos afectados.
> **Regla:** no se pasa de fase sin cerrar la anterior.
> **Regla:** cada decisión nueva se registra en `DECISION_LOG.md` antes de ejecutarse.

---

## Fase 0 — Asegurar

**Objetivo:** que nada se pierda otra vez.

**Pasos:**
1. Crear carpeta `libreria/` en `lenguaje-hermetico` con las 55 librerías.
2. Crear `libreria/README.md` con: lista de librerías, capa a la que pertenecen, resultado del test, fecha.
3. `git add` + `git commit` de las 55.
4. `git tag pre-reforma` en `lenguaje-hermetico` y en `motor-grafico`.
5. Copiar `DECISION_LOG.md` y `PLAN.md` (este archivo) a la raíz de ambos repos.

**Criterio de aceptación:**
- `git ls-files libreria/` muestra 55+ archivos.
- `git tag` muestra `pre-reforma` en ambos repos.

**Archivos afectados:** solo `libreria/` y raíz de cada repo. Cero cambios de código.

---

## Fase 1 — Decisiones

**Objetivo:** cerrar DL-07 a DL-14 y escribirlas en `DECISION_LOG.md`.

**Pasos:**
1. Revisar las decisiones `PENDIENTE` una por una.
2. Para cada una: elegir opción, motivo, consecuencias.
3. Actualizar `DECISION_LOG.md` con las decisiones tomadas.
4. Actualizar `PLAN.md` si alguna decisión afecta la secuencia de fases.

**Criterio de aceptación:**
- Cero decisiones `PENDIENTE` en `DECISION_LOG.md` (o las que queden, con fecha límite para decidirlas).

**Archivos afectados:** `DECISION_LOG.md`, `PLAN.md`.

---

## Fase 2 — Resolver

**Objetivo:** que las 55 librerías compilen y que la RAM bomb se resuelva.

**Pasos (en orden):**

### Paso 2.0 — Funciones faltantes en `herm_eval.cpp` (bug de corrección, PRIORIDAD ALTA).

**Objetivo:** que `evalExpression()` soporte todas las funciones que el lenguaje reconoce, y que falle ruidosamente ante funciones desconocidas.

**Contexto:** `herm_eval.cpp` solo reconoce 16 funciones. Cualquier otra devuelve `0.0f` silenciosamente. Eso corrompe silenciosamente:
- `herm_render.cpp` — PNG output.
- `herm_rih.cpp` — transforms y params por frame.
- `herm_ont.cpp:1073` — `d_min` del BVH (afecta rendimiento GPU).
- `herm_ont.cpp:564` — AABB de nodos CUSTOM (path hoy inactivo, bug latente).

**Causa raíz:** el fallback es silencioso (`return 0.0f`). Si fuera error, se habría detectado el primer día. "35/55 PASS" = 35/55 **compilan**, no 35/55 **correctas**. En CPU, cualquier lib usando `exp`/`acos`/`atan2`/`step`/`sign` produce valores incorrectos sin aviso.

**Sub-pasos:**

**2.0.1 — Añadir funciones faltantes.**
- `step`, `smoothstep`, `exp`, `log`, `log2`, `log10`, `acos`, `asin`, `atan`, `atan2`, `sign`, `mod`, `fract`.
- Tests unitarios de cada una contra valores conocidos.

**2.0.2 — Cambiar fallback a error.**
- `return 0.0f` → `throw std::runtime_error("herm_eval: función no soportada: " + name)`.
- Verificar que ningún test existente se rompe (si alguno usaba funciones no soportadas, ese test estaba mal).

**2.0.3 — Test de paridad CPU/GPU.**
- Para cada librería en `libreria/`, compilar una escena que use sus funciones y comparar valores CPU vs GPU.
- Sin tolerancia. Divergencia = error.

**2.0.4 — Re-ejecutar benchmarks obsoletos.**
- Marcar como obsoletos todos los `--render` anteriores.
- Re-ejecutar con el evaluador corregido.
- Comparar: los resultados deben ser distintos (más correctos).

**Criterio de aceptación de Paso 2.0:**
- Las 13 funciones nuevas se evalúan correctamente en CPU.
- Función desconocida → error explícito, no 0.
- Test de paridad CPU/GPU pasa para las 55 librerías.
- Los benchmarks de `--render` re-ejecutados son válidos.

**Archivos afectados:**
- `lenguaje-hermetico/herm/herm_eval.cpp`
- `lenguaje-hermetico/herm/herm_eval.h` (si cambia la firma)
- `tests/test_eval_parity.cpp` (nuevo)

**Prioridad:** antes de DL-09 (Paso 2.3–2.5). Es un bug de corrección, no de rendimiento. **Incorrecto > lento.** Independiente de DL-14. Se puede hacer en paralelo con las libs.

**Disciplina derivada:** todo fallo debe ser ruidoso. Si algo no se soporta → error. Si algo no existe → error. Si algo diverge → error. El silencio es deuda invisible.

---

**Paso 2.1 — Fix `fn` multi-statement.**
- Extender `parse_fn` (`herm_parser.cpp:297`) para aceptar `let` internos.
- Sintaxis: último statement del cuerpo es el valor de retorno.
- Aceptación: las 17 librerías con `fn` multi-statement compilan.

**Paso 2.2 — Fix keywords como parámetros.**
- Extender `expect(IDENT)` para aceptar tokens reservados en posición de parámetro.
- O reescribir 3 librerías.
- Aceptación: `std_bridge_electrochem`, `std_bridge_genomics`, `std_bridge_statmech_mol` compilan.

**Paso 2.3 — Constant folding real.**
- En `Resolver::toExpr`, evaluar la expresión si todos los argumentos son constantes.
- Aceptación: `let RADIO = 1.0; let ALTURA = RADIO * 1.5;` produce `let ALTURA = 1.5`.

**Paso 2.4 — Memoización de `fn`.**
- Cache de `(fn_name, arg_text)` → `expanded_text` en `expandExpression`.
- Aceptación: llamadas idénticas a `fn` no re-expanden.

**Paso 2.5 — CSE.**
- Detección de subexpresiones idénticas en el árbol expandido. Reemplazo por referencias.
- Aceptación: `test 6 librerías + 31 let` corre en memoria razonable (< 100 MB).

**Paso 2.6 — Rebuild de `herm.exe`.**
- Compilar con `build_libherm.bat`.
- Aceptación: `herm.exe` funciona. Las 55 librerías compilan.

**Criterio de aceptación de Fase 2:**
- 55/55 compilan.
- El test combinado corre sin RAM bomb.
- Ninguna librería requiere reescritura manual.

**Archivos afectados:** `herm_parser.cpp`, `herm_resolver.cpp`, `herm_eval.cpp` (si aplica).

---

## Fase 3 — Pipeline semántico

**Objetivo:** que el motor tenga acceso a la semántica que `.ont` destruye.

**Pasos (asumiendo DL-08 = B: sidecar `.sem`):**

**Paso 3.1 — Diseñar el formato `.sem`.**
- Definir estructura binaria o JSON.
- Campos mínimos: names, `def_id`, `parent_id`, `depends_on_w` por expresión, `spacing`, `displace_func`, hash de SDF por nodo.
- Escribir `docs/ont_format_v2.md` o `docs/sem_format_v1.md`.

**Paso 3.2 — Emitir el `.sem` desde `herm.exe`.**
- Extender `writeOnt` para escribir también `.sem`.
- Aceptación: cada `.ont` generado tiene su `.sem` asociado.

**Paso 3.3 — Leer el `.sem` desde el motor.**
- Añadir `OntScene::loadSem(const FileMapping& fm)`.
- Aceptación: el motor puede cargar el `.sem` y consultar sus campos.

**Paso 3.4 — Usar el `.sem` en el optimizador.**
- `ri_optimizer.cpp` consulta el `.sem` para:
  - Detectar nodos estáticos (`depends_on_w=false`).
  - Factorizar instancias (`def_id` idénticos).
  - Detectar subexpresiones comunes (hash de SDF).
- Aceptación: el optimizador hace más folding que antes (medible con `OptStats`).

Paso 3.5 (nuevo) — Diseñar el formato `.bake`.
- Definir estructura.
- Definir resolución por defecto.
- Definir compresión.
- Escribir `docsbake_format_v1.md`.

Paso 3.6 (nuevo) — Emitir `.bake` desde `herm.exe`.
- Comando `herm.exe --bake scene.herm` genera `scene.bake`.
- Solo si la escena tiene nodos estáticos.

Paso 3.7 (nuevo) — Leer `.bake` desde el motor.
- `OntSceneloadBake(const FileMapping& fm)`.
- El shader lo usa si existe.

**Criterio de aceptación de Fase 3:**
- Una escena con `teclado.herm` (38 teclas idénticas) reduce su `.ont` a 1 SDF + 38 transforms.
- El optimizador reporta instancias factorizadas.

**Archivos afectados:** `herm_ont.cpp`, `herm_bridge.cpp`, `render/scene.cpp`, `render/ri_optimizer.cpp`.

---

## Fase 4 — Contrato

**Objetivo:** decidir y ejecutar DL-07.

**Pasos (asumiendo DL-07 = B: contrato canónico):**

**Paso 4.1 — Mapear qué del contrato se puede migrar al motor.**
- `herm::sdf` → reemplaza `core/sdf_eval.cpp` y partes de `render/sdf_eval.cpp`.
- `herm::scene` → reemplaza `render/scene.cpp`, `scene/scene_graph.h`, `scene/scene_query.h`.
- `herm::material` → reemplaza el catálogo de materiales.
- `herm::ont` → reemplaza `mg::OntHeader` en `render/scene.h`.

**Paso 4.2 — Migrar pieza por pieza, con tests.**
- Empezar por `herm::sdf` (más independiente).
- Seguir por `herm::material`.
- Seguir por `herm::scene`.
- Terminar por `herm::ont`.

**Paso 4.3 — Eliminar código duplicado del motor.**
- `core/sdf_eval.cpp`, `core/renderer.cpp`, `core/rih_reader.cpp`, `core/core.h`.
- Partes de `render/scene.cpp`.
- `scene/scene_query.h` (el BVH del contrato lo reemplaza).
- `rhi/` completo.

**Criterio de aceptación de Fase 4:**
- El motor reduce LOC en al menos 2000.
- Todos los tests pasan.
- El binario `visor.exe` funciona igual que antes de la migración.

**Archivos afectados:** muchos. Se detalla en cada paso.

---

## Fase 5 — Optimizaciones semánticas

**Objetivo:** implementar las O1–O5.

**Pasos:**

**Paso 5.1 — Detección static/dynamic.**
- Usar `depends_on_w` del `.sem`.
- Separar geometría estática de dinámica en el shader.

**Paso 5.2 — Instanciación por `def_id`.**
- El shader compila una SDF por `def_id`.
- Las instancias solo aportan transform.

**Paso 5.3 — AABBs correctos para nodos animados.**
- Corregir `evalExprConst` (`herm_bridge.cpp:270`): no convertir expresiones `w`-dependientes a `0.0f`.
- Calcular AABB del rango de la animación.

**Paso 5.4 — Normales analíticas.**
- Diferenciar simbólicamente las expresiones SDF.
- Reemplazar diferencias finitas en el shader.

**Paso 5.5 — Implementar `REPEAT`, `TWIST`, `ELONGATE`, `DISPLACE`.**
- Usar `herm::sdf::opRepeat`, `opTwist`, `opElongate` del contrato.
- Eliminar los TODOs de `herm_bridge.cpp:513-528`.

**Paso 5.6 — CSE de subexpresiones en el shader.**
- Extender `glsl_gen.cpp` para detectar subexpresiones idénticas entre nodos.

Paso 5.7 (nuevo) — Sombras con `.bake`.
- El shader lee visibilidad pre-bakeada.
- Runtime para dinámicas.

Paso 5.8 (nuevo) — Difusa con `.bake`.
- El shader lee irradiancia pre-bakeada.
- Runtime para dinámicas.

Paso 5.9 (nuevo) — AO con `.bake`.
- Lectura directa.

Paso 5.10 (nuevo) — Lipschitz por nodo.
- El compilador propaga L por el árbol.
- El shader lo usa para pasos más grandes.

Paso 5.11 (nuevo) — Intersección analítica para primitivas puras.
- El compilador detecta esferas, cajas, cilindros puros.
- El shader genera código directo.

Paso 5.12 (nuevo) — Interval culling.
- Cotas inferioressuperiores por subárbol.
- Saltar intervalos vacíos.

Paso 5.13 (nuevo) — Gradiente analítico.
- Newton-Raphson en el último paso.
- Menos iteraciones de convergencia.

### Paso 5.14 — Intersección analítica para composiciones de primitivas puras.

**Objetivo:** extender la intersección analítica de Fase 5.11 (primitivas puras individuales) a composiciones de primitivas puras que también tienen solución cerrada. Eliminar el ray marching para `union`/`intersect`/`subtract` de primitivas puras.

**Dependencias:**
- Paso 5.1–5.5 cerrados (static/dynamic, `def_id`, AABBs, normales, operadores SDF).
- Fase 3 cerrada (`.sem` con `analytic_flags`).
- DL-09 cerrado (fold + memo + CSE).

**Sub-pasos (en orden estricto):**

**5.14.1 — Añadir `RayHit` y funciones de intersección a `contrato/sdf.h`.**
- `struct RayHit { bool hit; float t; Vec3 normal; };`
- `RayHit intersectSphere(Ray r, Vec3 center, float radius);`
- `RayHit intersectBox(Ray r, Vec3 min, Vec3 max);`
- `RayHit intersectCylinder(Ray r, Vec3 base, Vec3 axis, float radius, float height);`
- Tests unitarios de cada función contra casos conocidos.

**5.14.2 — Detección de subárboles analíticos en el compilador.**
- En `herm_ont.cpp` (o `herm_bridge.cpp`), al compilar cada nodo, recorrer el subárbol y clasificar:
  - `ANALYTIC_FULL`: todas las hojas son primitivas puras y todos los operadores son `union`/`intersect`/`subtract`.
  - `ANALYTIC_PARTIAL`: mezcla de analítico y no analítico.
  - `NON_ANALYTIC`: contiene `smooth_union` con `k>0`, `repeat`, `twist`, `elongate`, `displace`.
- Tests con escenas de prueba que cubran los tres casos.

**5.14.3 — Emitir flags en el `.sem`.**
- Añadir a `analytic_flags`:
  - `bit 0`: primitiva pura individual (ya en Fase 5.11).
  - `bit 1`: subárbol analítico completo.
  - `bit 2`: subárbol analítico parcial.
  - `bit 3`: tiene gradiente analítico.
- Verificar que el `.sem` se emite correctamente para escenas de prueba.

**5.14.4 — Generar `intersectLeaf` en `glsl_gen.cpp`.**
- Para nodos con `bit 1`: generar `intersectLeaf(node_idx, ray)` que resuelve analíticamente. Sin `evalLeaf` para ese nodo.
- Para nodos con `bit 2`: generar `intersectLeaf` que resuelve las hojas analíticas y hace ray marching sobre las no analíticas.
- Para nodos sin `bit 1` ni `bit 2`: seguir con `evalLeaf` (ray marching).
- Tests de compilación del shader. Verificar que el shader compila y produce resultados correctos.

**5.14.5 — Integrar en `bvhEval`.**
- Añadir `bvhIntersect(ray)` que, para nodos analíticos, resuelve directamente; para el resto, delega en `bvhEval`.
- El dispatcher decide según los flags del `.sem`.
- Tests visuales: comparar render con y sin la optimización. Deben ser idénticos (la intersección analítica es exacta).

**5.14.6 — Medir rendimiento.**
- Benchmark antes/después en escenas con:
  - Muchas esferas puras.
  - Muchas cajas puras.
  - Composiciones `union` de primitivas puras.
  - Composiciones `intersect` de primitivas puras.
  - Composiciones `subtract` de primitivas puras.
  - Escenas con `smooth_union` con `k>0` (deben seguir igual, sin ganancia).
  - Escenas con `displace` (deben seguir igual, sin ganancia).
- Documentar ganancia real por tipo de escena.

**Criterio de aceptación de Paso 5.14:**
- Una escena con 10 esferas puras renderiza sin ray marching (verificable con contador de pasos en el shader).
- Una escena con `union(esfera, caja)` renderiza sin ray marching.
- Una escena con `subtract(caja, esfera)` renderiza sin ray marching.
- Una escena con `smooth_union(esfera, caja, 0.5)` sigue usando ray marching (correcto).
- Ganancia medida vs. estado actual: 5–20× en escenas con muchas primitivas puras.
- Sin artefactos visuales. Las normales se calculan analíticamente.
- El fallback a ray marching funciona siempre. Ninguna escena deja de renderizar.

**Archivos afectados:**
- `contrato/sdf.h` — añadir `RayHit` y funciones de intersección.
- `deps/lenguaje-hermetico/herm/herm_ont.cpp` — detección de subárboles analíticos.
- `render/glsl_gen.cpp` — generación de `intersectLeaf`.
- `render/scene.h` — leer `analytic_flags` (del `.sem` o del `.ont`).
- `core/herm_bridge.cpp` — propagar flags al `.ont` o al `.sem`.
- `tests/test_f5_14_*.cpp` — tests nuevos.

**Lo que NO se hace:**
- No se resuelve `smooth_union` con `k>0`.
- No se resuelve `displace`, `twist`, `elongate`.
- No se reemplaza el ray marching. Se complementa.
- No se toca el lenguaje. Los flags son del compilador y del motor.

**Riesgos:**
- **R1.** Detección conservadora. Aceptable: fallback es ray marching.
- **R2.** El shader crece. Más casos que testear. Mantener fallback siempre funcional.
- **R3.** La ganancia depende de la escena. Medir antes de prometer.
- **R4.** Requiere `.sem` implementado. Sin eso, no hay flags.

**Criterio de aceptación de Fase 5:**
- `teclado.herm` con 38 teclas renderiza con 1 SDF en el shader.
- Escenas con `displace` funcionan.
- Benchmarks de rendimiento mejoran respecto a Fase 3.
- Escenas con composiciones de primitivas puras renderizan sin ray marching. Ganancia medida de 5–20× en esas escenas. Fallback a ray marching siempre funcional.

**Dependencias de Fase 5:** Fase 3 (`.sem`). Paso 5.14 depende además de DL-09. No se puede implementar antes. Si el harness intenta hacerlo, parar y consultar.

**Archivos afectados:** `render/glsl_gen.cpp`, `render/spirv_gen.cpp`, `render/ri_optimizer.cpp`, `core/herm_bridge.cpp`.

---

## Fase 6 — Validación end-to-end

**Objetivo:** una escena real que use las 55 librerías, con benchmarks.

**Pasos:**

**Paso 6.1 — Construir una escena de prueba.**
- Que use al menos 5 librerías del árbol ontológico.
- Que use `def`/`instance` con parámetros.
- Que tenga nodos animados y estáticos.

**Paso 6.2 — Benchmark antes/después.**
- Medir FPS, VRAM, RAM.
- Comparar con el estado pre-reforma.

**Paso 6.3 — Tests de correctitud.**
- Verificar que las librerías físicas producen valores numéricos correctos.
- Comparar contra las ecuaciones científicas originales.

**Paso 6.4 — Documento final.**
- `docs/RESULTADOS.md` con benchmarks, decisiones tomadas, deuda pendiente.

**Criterio de aceptación de Fase 6:**
- Una escena que integra 5+ librerías.
- Benchmarks documentados.
- Tests de correctitud pasando.
- Documento final escrito.

---

## Reglas del plan

1. **Ninguna fase se abre sin cerrar la anterior.**
2. **Ninguna decisión se ejecuta sin estar en `DECISION_LOG.md`.**
3. **Ningún commit se hace sin actualizar el log si hubo decisión.**
4. **Ningún archivo nuevo se añade sin justificar por qué el contrato no lo cubre.**
5. **Si aparece una decisión nueva a mitad de fase, se registra y se decide antes de seguir.**

---

## Estado actual

- **Fase 0:** cerrada — tag `pre-reforma`, 55 librerías migradas (v0.15). La cola de reescritura se cerró el 2026-11-04 (55/55 compilan, v0.20).
- **Fase 1:** cerrada — DL-07 a DL-21 decididas y registradas (v0.16–v0.18). DL-22 a DL-25 son decisiones de Fase 5, marcadas PENDIENTE a propósito.
- **Fase 2:** es la siguiente. **No se abre hasta aprobación explícita del usuario.** Primer paso: **Paso 2.0** (13 funciones faltantes en `herm_eval.cpp` + fallback silencioso convertido en error), después DL-09 (fold + memo + CSE).
- **Fases 3–6:** pendientes.
- **Decisiones abiertas:** DL-23, DL-24, DL-25 (Fase 5); DL-28 (`ior`/`transmission` en `material`); DL-30 (opcodes ONT / rechazo ruidoso).

---

## Nuevo orden de dependencias

```
DL-08 (preservar semántica)
    ↓
DL-15 (formato .bake)
    ↓
Fase 3 (pipeline semántico con .sem + .bake)
    ↓
Fase 5 (optimizaciones, incluidas sombrasdifusaAO)
```

DL-15 depende de DL-08. Sin semántica preservada, no hay forma de saber qué es estático.

Fase 5 depende de Fase 3. Sin `.bake`, no hay optimizaciones de lucessombras.

---

## Lo que hay que decidir ahora vs después

Ahora (bloqueante)
- DL-08 cómo preservar semántica. Sin esto, no hay `.bake`.

En Fase 1 (decisiones)
- DL-15.a hasta DL-15.e sub-decisiones del `.bake`.

En Fase 3 (implementación)
- Formato `.bake` concreto.
- Resolución y compresión.

En Fase 5 (optimizaciones)
- Cuáles de las 7 técnicas implementar primero.
- Medir cada una. Sin medición, no se sabe qué da más.

---

## Lo que no se puede prometer

- Que el `.bake` quepa en VRAM de GPUs modestas. Hay que medir. Si no cabe, la técnica no aplica a todo hardware.
- Que el bake tarde poco. Puede tardar minutos. Hay que aceptarlo o descartar la técnica.
- Que la ganancia sea la que digo. Son estimaciones. Hay que medir antes de decidir cuánto invertir.

Lo que sí se puede prometer
- El análisis simbólico del SDF es posible. Tu lenguaje lo permite.
- Hay ganancias reales en luces y sombras con pre-cálculo.
- El formato `.bake` es la forma correcta de organizar esa información.
- Diseñarlo ahora evita reescribir `.ont`, `.sem`, el shader y el pipeline después.
