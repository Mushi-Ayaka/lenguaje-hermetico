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
- `sphere(1.0 + x)`, `sphere(1.0 + exp(x))` y `sphere(1.0 + step(0.5, x))` generan **bytecode idéntico**. La causa exacta es `cf()` (`herm_ont.cpp:47`): si `Expr::is_expr` es true devuelve el valor por defecto (`1.0` para el radio) y no emite nada. El `VAR_X` que aparece en el bytecode es la fórmula del propio `sphere`, no la expresión del parámetro. El compilador no avisa.

**Consecuencia:** cualquier expresión en un parámetro SDF se renderiza en GPU con el valor por defecto, sin error. No es solo un problema de 12 funciones: `sphere(1.0 + x)`, `sphere(1.5 * 0.5)` y `sphere(exp(x))` se comportan igual. La paridad exigida en el Paso 2.0c **no puede pasar** hasta que existan los opcodes y, además, un compilador de expresiones a bytecode (o hasta que el compilador rechace esas expresiones).

**Segundo desajuste:** `ONT_DIV` y `ONT_MOD` (`render/bytecode_vm.h:59-60`) devuelven 0 cuando el divisor es 0, mientras el CPU ya usa IEEE (`inf`/`nan`, v0.21). Divergencia garantizada en ese caso.

**Opciones:**
1. **Rechazo ruidoso ya** en el compilador ONT: si una expresión usa una función sin opcode, error con nombre y línea. Barato, sin cambio de formato, y convierte un render incorrecto en un fallo visible. No da paridad, pero elimina el silencio.
2. **Extender el bytecode** con 12 opcodes nuevos (`step, smoothstep, exp, log, log2, log10, acos, asin, atan, atan2, sign, fract`) y actualizar los 5 consumidores: `bytecode_vm.h`, `ray_march_simd.h`, `glsl_gen.cpp`, `spirv_gen.cpp`, `jit_compiler.cpp`. Requiere decisión de versión del formato (¿v2? ¿campo `reserved`?) y de semántica exacta para paridad (p. ej. `mod` GLSL vs `fmod`).
3. **Limitar el lenguaje** a las funciones que el bytecode ya soporta y retirar las 13 del CPU. Contradice 2.0b.

**Recomendación del agente:** 1 ahora mismo (es un fallo ruidoso, no un cambio de arquitectura) y 2 como paso propio con su decisión de formato. Sin 1, cualquier test de paridad mide una mentira.

---

## DL-31 — 2.0c.1: rechazo ruidoso de parámetros SDF que el bytecode ONT no puede representar.

**Estado:** IMPLEMENTADO en v0.22 (2026-11-07; mandato del usuario del 2026-11-04). Secuencia 2.0c.1 → 2.0c.2 → 2.0c.3 → 2.0c.4.

**Contexto (corrige el alcance de DL-30):** el compilador `.ont` no compila **ninguna** expresión de parámetro SDF. `sdfPrimToBytecode` usa `cf(i, def)` (`herm_ont.cpp:47`), que devuelve `def` cuando `n.params[i].is_expr` es true. No es únicamente que falten opcodes para 12 funciones: `sphere(1.0 + x)` también se convierte en `sphere(1.0)`. `SdfType::CUSTOM` emite `ONT_CONST, 1e9` (un "miss" silencioso), y `smooth_union`, `smooth_subtract`, `repeat`, `twist`, `elongate` y `displace` degradan sin avisar. Este paso cierra el silencio de los parámetros SDF; las degradaciones de operadores se registran aparte y no se tocan aquí.

**Decisión (2.0c.1):**
- Sin cambio de formato `.ont`. Sin opcodes nuevos.
- Si un parámetro SDF es una expresión, `herm.exe --ont` falla con `std::runtime_error`: primitiva, índice de parámetro, línea del archivo y, si se detecta, la función sin opcode.
- `SdfType::CUSTOM` se rechaza explícitamente: hoy renderiza un "miss" silencioso.
- El error se propaga hasta `main`, que ya captura `std::exception`, imprime `Error: ...` y devuelve código distinto de 0.

**Implementación (solo memoria, sin serializar):**
- `ExprNode` (`herm_ast.h`) y `Expr` (`contrato/rih.h`) ganan `int line = 0`. No se escribe en `.rih` (JSON ni binario) ni en `.ont`; no cambia el contrato de archivos.
- `Parser::parse_expr` registra la línea inicial de la expresión.
- `Resolver::toExpr` propaga `line`.
- `sdfPrimToBytecode` rechaza `is_expr` y `CUSTOM` con nombre y línea.

**No entra en este paso:** compilar expresiones a bytecode, añadir los 12 opcodes, decidir versión de formato (2.0c.2) ni semántica de división (2.0c.4).

**Medición (2026-11-07).** Tres pruebas independientes, ejecutadas sobre el árbol de v0.22.

**P1 — estática, sobre las 55 librerías.** Se recorren los cuerpos de todos los `fn` y se busca una llamada a `step`, `smoothstep`, `exp`, `log`, `log2`, `log10`, `acos`, `asin`, `atan`, `atan2`, `sign` o `fract`.
- **46/55** la usan directamente en al menos un `fn` propio.
- **46/55** la alcanzan de forma transitiva (llamando a otras `fn`).
- **9/55 no la usan de ninguna forma:** `std_astro_cosmology`, `std_astro_mechanics`, `std_astro_stellar`, `std_bio_physiology`, `std_bio_systems`, `std_geo_landscape`, `std_geo_ocean`, `std_phys_kinematics`, `std_phys_relativity`.

**P2 — dinámica, 1070 funciones.** Por cada `fn` de cada librería se genera una escena `sphere(1.2345 + fn(1.2345, ...))` y se compila con `--ont --frames 0-0`. Se probaron **1070 funciones** (las que tienen firma regular).
- **261/1070 funciones fallan; 32/55 librerías tienen al menos una.**
- El número bruto **no** mide el coste de 2.0c.1. Separado por causa:
  - **13 funciones en 3 librerías** dependen de `x`/`y`/`z` en un parámetro SDF. Es el caso que 2.0c.1 rechaza. Librerías: `std_geo_terrain` (3), `std_phys_condensed` (5), `std_phys_sdf` (5). De las 13, **5 usan `exp`** (`bloch_wave`, `fcc_density`, `bcc_density`, `gravity_wave`, `breaking_wave`) y **8 son solo `x`/`y`/`z`** (`sediment_layer`, `volcano_profile`, `caldera_depth`, `bloch_envelope`, `cubic_lattice_potential`, `standing_wave3d`, `vibrating_membrane`, `wave_with_current`). Las 8 se podrían compilar con los opcodes actuales si el compilador emitiera la expresión del parámetro; las 5 necesitan además el opcode de `exp`.
  - **243 funciones en 29 librerías** fallan por una causa ajena a 2.0c.1: el lexer no reconoce la notación científica. `1e-6` se tokeniza como `1`, `e`, `-6`; `parse_expr` los une con espacios y `evalExpression` recibe `1 e -6` y lanza "expresion invalida (pos ...)". Se reproduce **sin `--ont`**, con `--frames 0-0`: no es una regresión de este paso (DL-33).
  - **5 funciones en 4 librerías** dan `inf` con los argumentos del probe (`1.2345`): `equilibrium_constant_from_potential`, `butler_volmer_current`, `activation_energy_from_arrhenius`, `flory_huggins_dG`, `saturation_pressure`. El rechazo es correcto; la fórmula desborda con esos valores.
  - Dentro de las 243, además de la notación científica hay **constantes no definidas** (`LN2`, `PLANCK`, `tanh`) que el evaluador no conoce.

**Conclusión de la medición.** La opción 1 sola rompe **3/55** librerías por parámetros SDF no representables, por debajo del umbral de 5 que el usuario fijó para descartarla. Pero eso no cierra el problema: 2.0c.1 solo convierte el silencio en error. Las 8 funciones puramente espaciales y las 5 con `exp` siguen sin poder compilarse, y el criterio de 2.0c.3 ("55/55 con valores idénticos CPU/GPU") **es inalcanzable mientras exista DL-33**, porque 29/55 librerías ni siquiera llegan al bytecode.

**P3 — formato real de `.ont`.** `sizeof(OntHeader)` es **120 bytes**, no 80 como afirma `render/ont_format_v1.md`. Layout verificado: `magic` 0, `version` 4, `epsilon` 8, `node_count` 12, `bvh_count` 16, `material_count` 20, `bytecode_size` 24, `tensor_buffer_size` 28, `scene_aabb_min[4]` 32–47, `scene_aabb_max[4]` 48–63, `reserved[7]` 64–119 (56 bytes). Comprobación empírica: un `.ont` de 423 B = 120 de cabecera + 52 de BVH + 116 de graph node + 75 de bytecode + 60 de material. `reserved` está libre y alineado; cualquier extensión futura cabe ahí sin cambiar el tamaño de la cabecera.

**Aceptación verificada (2026-11-07).** `sphere(1.0 + exp(x))` con `--ont` sale con código 1 y el mensaje `el parametro 0 de 'sphere' depende de x/y/z ('1.0 + exp ( x )') y el bytecode ONT no puede representarlo; funcion sin opcode: 'exp' (linea 8)`. No se escribe `.ont` ni `.obs`. Los casos plegables sí compilan: `1.5*0.5` da radio 0.75, `0.65 + sin(w*2)*0.35` da 0.65 en el fotograma 0, `exp(1.0)` da 2.7183. `1.0+x`, `1.0+step(0.5,x)` y `custom(y-...)` fallan ruidosamente.

---

## DL-32 — Fallbacks silenciosos que quedan en el compilador `.ont` y en `--render`.

**Estado:** MEDIDO (2026-11-07). La condición dura del usuario no se cumple: 0/55 librerías usan `smooth_union` o `displace`. Orden pendiente de confirmación; no se toca en 2.0c.1 ni en 2.0c.2.

DL-31 cierra el silencio de los **parámetros** SDF. Quedan cinco degradaciones que siguen renderizando otra cosa sin avisar:

| Caso | Qué hace hoy | Qué debería hacer |
|---|---|---|
| `smooth_union(a, b, k)` | emite `min(a, b)`; ignora `k` (`herm_ont.cpp:287`) | fallar, o emitir la fórmula suave con un opcode nuevo |
| `smooth_subtract(a, b, k)` | emite la resta sharp (`herm_ont.cpp:298`) | fallar |
| `smooth_intersect(a, b, k)` | emite `max(a, b)` | fallar |
| `repeat`, `twist`, `elongate`, `displace` | pass-through del hijo A (`herm_ont.cpp:307-310`) | fallar |
| `--render` con primitiva integrada | usa `params[i].constant` (`herm_render.cpp:152-160`); la expresión del parámetro nunca se evalúa | validar igual que `--ont` o evaluar por muestra |

El path `--render` es el más grave: `validateOntCompatibility` solo se llama con `emit_ont`. Una escena como `sphere(1.0 + x)` renderizada con `--render` (sin `--ont`) **sale con código 0 y una imagen incorrecta**. Con `--ont` ahora falla; con `--render` no.

**Prueba (2026-11-07, 64×64, `--render`):** `sphere(0.75)` produce un PNG de 862 B; `sphere(1.5 * 0.5)` y `sphere(0.75 + x)` producen **el mismo PNG de 257 B** (hash MD5 idéntico, `8591307...`). Es decir, cualquier expresión de parámetro se renderiza como `sphere(0)` — ni siquiera se pliega una expresión constante. Exit code 0 en los tres casos.

**Medición de alcance (2026-11-07).** Se recorrieron las 59 escenas del repositorio (46 de `libreria/tests` + 13 de `ejemplos`), resolviendo cada una a `.rih` y contando operadores, primitivas y expresiones de parámetro en el árbol SDF resultante.

- **Librerías (55):** **0** usan `smooth_union`, `smooth_subtract`, `smooth_intersect`, `repeat`, `elongate` o `displace` como operador. `twist` aparece en dos líneas, ambas comentarios (`std_bridge_molbio.hm:56`, `std_chem_supramolecular.hm:252`); no hay ninguna llamada.
- **Escenas:** 49 resuelven a `.rih`, 10 fallan por causas preexistentes (`translate` en `test_phys_sdf`, `ior` en `bodegon`, etc.). De las 49:
  - **1** usa un operador DL-32: `libreria/tests/test_physics_lib.herm`, nodo `complex_shape_2`, `smooth_union(sphere(0.25), sphere(0.25), k=3.0)`.
  - **2** usan `plane`: `ejemplos/def_let` y `ejemplos/luces`. **0** usan `capsule` o `cone`.
  - **20** tienen expresiones de parámetro, con **106 parámetros** en total: **6 vectoriales** (`[ ... ]`), **85 espaciales** (`x`/`y`/`z`), **2 temporales** (`w`) y **13 constantes**. `--render` ignora los 106 (dibuja `params[i].constant = 0`); `--ont` rechaza los 91 espaciales/vectoriales con 2.0c.1 y pliega los 15 constantes/temporales.

**Conclusión de la medición.** La condición dura del usuario —"si las librerías usan `smooth_union` o `displace`, DL-32 va antes de 2.0c.3"— **no se cumple**: 0/55 librerías. Pero `--render` afecta a **20/49 escenas** y la única escena con `smooth_union` sí resuelve. La decisión de si DL-32 va antes o después de 2.0c.3 queda pendiente de confirmación del usuario; el agente no la toma.

**Hallazgo adicional (misma auditoría, 2026-11-07):** aunque los parámetros sean constantes, tres primitivas no significan lo mismo en el bytecode que en el render CPU:
- `plane`: el bytecode emite `y + d` usando `params[0]` como distancia (`herm_ont.cpp:185-189`); el render CPU usa `params[0..2]` como normal y `params[3]` como distancia (`herm_render.cpp:157`).
- `capsule`: el bytecode emite una esfera de radio `params[0]` e ignora la altura (`herm_ont.cpp:162-170`).
- `cone`: el bytecode usa `max(sqrt(x²+z²) - r*(1-y/h), y)` (`herm_ont.cpp:190-201`), que no es el `sdCone` del render CPU.

Esto también rompe la paridad y no lo arregla ningún opcode nuevo. Debe entrar en el alcance de 2.0c.3 o registrarse como paso propio.

**Opcodes desconocidos (relevante para 2.0c.2):** los cinco consumidores reaccionan de forma silenciosa ante un opcode que no conocen: el VM escalar hace `default: break` (`bytecode_vm.h:83`), el SIMD devuelve el resultado parcial (`ray_march_simd.h:205`), GLSL y SPIR-V empujan `"0.0"` (`glsl_gen.cpp:360`, `spirv_gen.cpp:100`) y el JIT hace `default: break` (`jit_compiler.cpp:380`). Si se añaden opcodes sin tocar esto, un lector viejo convierte la instrucción nueva en 0 sin avisar. Cualquier decisión de formato debe incluir el endurecimiento de los cinco.

**Opciones para 2.0c.3:** (a) rechazar los seis operadores y el path `--render` hasta que exista opcode, o (b) implementarlos en el bytecode. La decisión corresponde al usuario; este registro no la toma.

---

## DL-33 — El lexer no acepta notación científica; 29/55 librerías no llegan al bytecode.

**Estado:** IMPLEMENTADO Y VALIDADO (2026-11-08, v0.24). Orden explícito del usuario: después de 2.0c.2 y **antes** de 2.0c.3; no en paralelo.

**Bug:** `Lexer::read_number` (`herm_lexer.cpp:263-278`) lee signo, dígitos y punto decimal, pero no `e`/`E` con exponente. `1e-6` se tokeniza como `1`, `e`, `-6`. `Parser::parse_expr` une los tokens con espacios (`herm_parser.cpp:95-118`) y produce `1 e -6`. `evalExpression` no reconoce `e` como número y lanza `expresion invalida (pos ...)`.

**Alcance medido (2026-11-07):** 243 de las 1070 funciones del probe P2 (29/55 librerías) fallan por esta causa. Se reproduce **sin `--ont`** con `--frames 0-0`, así que no es una regresión de 2.0c.1: cualquier path que pliegue expresiones (`--frames`, `--render`, `--ont`) tropieza con ella. La librería `std_bio_genetics.hm` es el caso mínimo (`... + 1e-6`).

**Casos relacionados, misma prueba:** `1e6` se convierte en `1` + identificador `e6` (constante desconocida); `LN2`, `PLANCK` y `tanh` tampoco existen en `herm_eval.cpp`. Son 5 funciones adicionales dentro de las 243.

**Consecuencia para 2.0c.3:** el criterio "las 55 librerías dan valores idénticos CPU/GPU" no se puede cumplir con opcodes: 29/55 fallan antes de emitir bytecode. Hay que decidir si se arregla el lexer (bugfix pequeño, sin sintaxis nueva) y se añaden las constantes/funciones que faltan **antes** de 2.0c.3, o si el criterio se limita a las librerías que hoy son representables. Recomendación del agente: arreglar el lexer y las constantes como paso 2.0c.2b, antes de los opcodes.

**Solución implementada (2026-11-08):**
- `Lexer::read_number` acepta `e`/`E` con signo y dígitos. Solo consume el exponente si está bien formado; `1e` y `1e+` siguen fallando con `expresion invalida`.
- `evalExpression` añade las constantes `LN2` y `PLANCK` y la función `tanh`.

**Resultado de la medición (2026-11-08, mismo probe P2):**
- **Antes:** 261/1070 fallan en 32/55 librerías; **243 en 29** por DL-33.
- **Después:** **45/1070 fallan en 18/55 librerías; 0 por notación científica.**
- Restantes: **18** expresiones vectoriales usadas en un parámetro escalar (limitación del probe), **14** `no finito` por los argumentos fijos del probe, **13** parámetros espaciales reales (2.0c.3). La suma da 45 porque un fallo espacial (`bloch_wave`) menciona además `exp` sin opcode.
- Verificado: `2.5e+3` → 2500, `1E6` → 1000000, `5e-1` → 0.5, `1e-6` compila; `LN2 * 2.0` → 1.3863, `tanh(1.0)` → 0.7616, `LN2 * 2.0 + PLANCK * 1e34` → 8.0124.

**Consecuencia para 2.0c.3:** el criterio corregido del usuario ("todas las que compilan producen paridad") ya es medible: 18/55 librerías tienen al menos una función que aún falla por causas ajenas a DL-33, y 13 de esos fallos son parámetros espaciales que 2.0c.3 debe resolver.

---

## DL-34 — 2.0c.2: formato y versionado de los opcodes nuevos (opción A).

**Estado:** IMPLEMENTADO Y VALIDADO (2026-11-07, v0.23). Se implementó antes de 2.0c.2b. No añade opcodes nuevos: solo instala el formato, el versionado y el rechazo ruidoso. Los opcodes concretos son 2.0c.3.

**Decisión (opción A):**
- **La cabecera sigue en 120 bytes.** No se mueve ningún offset. `reserved[0]` (8 bytes) pasa a ser `feature_flags`; `reserved[1..6]` siguen reservados y deben ser 0.
- **`version = 1`** si el bytecode solo usa opcodes v1 (0..`ONT_END`). **`version = 2`** solo si el bytecode usa al menos un opcode nuevo (`> ONT_END`).
- **Opcodes nuevos detrás de `ONT_END`** (36..47). Los `.ont` v1 existentes se leen igual; no hay migración de assets.
- **`feature_flags`:** bit 0 = math extendida (`exp`, `log`, `step`, `smoothstep`, `sign`, `fract`, ...); bit 1 = parámetros SDF como expresión. Los bits no definidos se rechazan.
- **`loadOnt`:** acepta `version == 1` siempre; `version == 2` solo si todos los flags son conocidos; rechaza `version >= 3`, flags desconocidos y `reserved[1..6] != 0`.

**Tres condiciones duras del usuario (criterio de aceptación, no nota):**
1. **Fallo ruidoso en los 5 consumidores** ante un opcode desconocido: VM escalar (`bytecode_vm.h`), SIMD (`ray_march_simd.h`), GLSL (`glsl_gen.cpp`), SPIR-V (`spirv_gen.cpp`) y JIT (`jit_compiler.cpp`). Hoy los cinco lo convierten en 0 o devuelven un resultado parcial.
2. **`loadOnt` rechaza `version >= 3` y `feature_flags` desconocidos.** No basta con aceptar cualquier versión como hoy (`version < 1`).
3. **`version = 2` se emite solo cuando el bytecode usa opcodes nuevos.** Un `.ont` con solo opcodes v1 sigue siendo v1.

**Criterio de 2.0c.3 corregido (mandato del usuario, 2026-11-07):** "**todas las librerías que compilan producen paridad CPU/GPU**", no "55/55". El 55/55 es el objetivo final de la fase, no el criterio de aceptación de 2.0c.3.

**Compatibilidad:** el compilador (`herm`, `contrato/ont.h`) y el motor (`render/scene.h`) deben compartir los mismos valores de flags, versión máxima y rango de opcodes nuevos. El puente `core/herm_bridge.cpp` también calcula versión/flags al construir un `OntScene` en memoria.

**Aceptación de 2.0c.2:**
- Un `.ont` v1 con `reserved[0] = 0` se lee igual que antes.
- Un `.ont` con `version = 3` se rechaza con mensaje explícito.
- Un `.ont` v2 con un bit de flags no definido se rechaza con mensaje explícito.
- Cualquiera de los 5 consumidores, ante un opcode desconocido, falla con mensaje (excepción o `GlslGenResult.ok = false`), no devuelve 0.
- Un `.ont` generado por el compilador actual sigue saliendo con `version = 1`; el escaneo de bytecode confirma que no hay opcodes nuevos.

**Resultado de la validación (2026-11-07):**
- `.ont` de `ejemplos/teclado.herm`: `version = 1`, `reserved[0..6] = 0`, 1126 bytes.
- Lector (`OntScene::loadOnt`): **14/14** — carga v1; rechaza v3 (`version 3 no soportada`), v2 con flags desconocidos (`0x4`), v1 con flags no cero, opcode 200 (`nodo 0: opcode 200 desconocido`) y archivo truncado (`tamano ... != esperado`); acepta v2 con flags conocidos.
- Los 5 consumidores: **10/10** — VM escalar, VM SIMD, GLSL, SPIR-V y JIT lanzan/retornan error con `opcode 200` en el mensaje.
- Suites del motor: f0 **24/24**, f1 **10/10**, f3 **16/16**, scene **81/82** (`orbit zoom reduces distance`, fallo preexistente), os_rhi **30/31** (fallo preexistente).
- `test_ont_bridge` con `teclado.herm`: OK (43 nodos, 1 BVH, 3 materiales, 310 bytes de bytecode).

**Hallazgo durante la validación:** el compilador anterior escribía `OntHeader` sin inicializar; `reserved[1..6]` contenía basura del stack (`0x00007FF7302B0595`, etc.). El nuevo `OntHeader hdr = {}` lo corrige. Como `loadOnt` solo valida `reserved[0]`, los `.ont` viejos siguen siendo aceptados.

---

## DL-35 — 2.0c.3a: contrato de paridad CPU/GPU (tolerancias ULP, `mod` y división).

**Estado:** IMPLEMENTADO (2026-11-08, v0.25; solo documentación, sin código). Este commit desbloquea 2.0c.3b; los opcodes no se implementan hasta que exista.

**Mandato del usuario (2026-11-08):**
- La referencia CPU del test de paridad es la **VM de bytecode**: `execBcRaw` (`render/bytecode_vm.h:21`) y `execBcRaw4` (`render/ray_march_simd.h:16`). No es `--render`.
- Orden estricto: **2.0c.3a → 2.0c.3b → 2.0c.3c → 2.0c.3d**. DL-32 va **después** de 2.0c.3, no antes.
- 2.0c.4 (división) solo si no se resuelve aquí.

**Por qué `--render` no es la referencia:**
- `evalSdfTree` lee `node.params[i].constant` en todas las primitivas (`herm_render.cpp:152-160`); solo `CUSTOM` (`:202`) y `displace` (`:220`) evalúan la expresión.
- `--render` usa el árbol SDF, no el bytecode; la GPU consume bytecode (`glsl_gen.cpp`, `spirv_gen.cpp`).
- Medido en DL-32: `sphere(0.75 + x)` y `sphere(1.5 * 0.5)` dan el mismo PNG de 257 B, exit 0; 20/49 escenas tienen expresiones de parámetro; `plane`/`capsule`/`cone` ya divergen entre bytecode y `--render` (DL-32:611-631).
- Por eso el test compara **el mismo bytecode** en CPU (VM) y GPU (GLSL/SPIR-V). `--render` queda fuera del test; su fallo ruidoso es DL-32, después.

**Tabla de tolerancias (fijada; se documenta también en `tests/test_eval_parity.cpp`):**

| Función | Tolerancia | Justificación |
|---|---|---|
| `step` | 0 ULP | Comparación; solo devuelve `0.0` o `1.0`. |
| `smoothstep` | 0 ULP | Polinomio `t*t*(3-2t)` con `t = clamp(...)`; misma secuencia en CPU y GPU. Si el driver usa FMA y difiere, 2.0c.3d lo mide y actualiza la tabla con una entrada nueva. |
| `sign` | 0 ULP | Comparación; `-1`, `0`, `+1`. |
| `mod` | 0 ULP | Fórmula GLSL `x - y*floor(x/y)`; prohibido `fmod`. |
| `fract` | 0 ULP | `x - floor(x)`; la parte fraccionaria es exacta para finitos. |
| `min`, `max`, `clamp`, `abs` | 0 ULP | Operaciones IEEE exactas; `min`/`max` con semántica GLSL (NaN devuelve el otro operando). |
| `sqrt` | 0 ULP | IEEE 754 exige sqrt correctamente redondeada; se asume que el driver cumple. Si no, fallo ruidoso. |
| `floor`, `ceil` | 0 ULP | Exactas. |
| `exp`, `log`, `log2`, `log10` | 1 ULP | Trascendentes; libm y GPU pueden diferir en el último bit. |
| `acos`, `asin`, `atan`, `atan2` | 1 ULP | Trascendentes; mismo motivo. |
| `tanh` | 1 ULP | Trascendente; mismo motivo. **Nota:** `tanh` no está entre los 12 opcodes de 2.0c.3b; la fila es política para cuando exista opcode. El test de 2.0c.3c no la cubre salvo que se apruebe un 13º opcode. |

**Comparación:** 0 ULP = bits idénticos para finitos, con `+0 == -0`, `NaN == NaN` e `inf` solo igual a `inf` del mismo signo. 1 ULP = distancia entera <= 1 en la representación ordenada de binary32, con los mismos casos especiales. Cualquier otra diferencia es error. No hay epsilon arbitrario.

**No se propone cambiar ninguna tolerancia:** no hay medición GPU que lo justifique. La tabla se fija tal cual; si 2.0c.3d encuentra una diferencia, se mide y se registra en una entrada nueva antes de tocar el test.

**Semántica de `mod` (fijada):** GLSL `x - y*floor(x/y)`, no C `fmod`.
- `herm_eval.cpp:208-209` ya la usa (evaluador del lenguaje).
- `bytecode_vm.h:62` y `ray_march_simd.h:102-107` usan `std::fmod` y devuelven `0` si `b == 0`; **incorrecto** para la referencia CPU. 2.0c.3b debe cambiarlos a la fórmula GLSL.
- `jit_compiler.cpp:29` usa `fmodf`; `:287` lo invoca. **Incorrecto** para negativos; 2.0c.3b debe cambiar el helper a la fórmula GLSL.
- `glsl_gen.cpp:327` y `spirv_gen.cpp:67` emiten `mod(a, b)`; la fórmula es correcta. La tolerancia 0 ULP aplica al resultado del driver.
- Vectores de prueba: `mod(-1,3)=2`, `mod(1,-3)=-2`, `mod(-1,-3)=-1`, `mod(3,2)=1`, `mod(-3,2)=1`, `mod(3,-2)=-1`, `mod(5.5,2)=1.5`, `mod(1,0)=NaN`, `mod(0,0)=NaN`.

**Semántica de división (fijada):** IEEE 754, `inf`/`NaN`; prohibido el 0-silencioso.
- `herm_eval.cpp:88` ya la usa (`r /= rhs`; comentario "IEEE: inf/NaN como en GPU").
- `bytecode_vm.h:61` devuelve `0` si `b == 0`; **incorrecto**. 2.0c.3b debe emitir `a / b` sin máscara.
- `ray_march_simd.h:95-100` enmascara `b == 0` a `0`; **incorrecto**. 2.0c.3b debe dejar `_mm_div_ps(a, b)`.
- `jit_compiler.cpp:173-188` enmascara `b == 0` a `0`; **incorrecto**. 2.0c.3b debe dejar `divps`.
- `glsl_gen.cpp:326` y `spirv_gen.cpp:66` emiten `(a / b)`. La spec GLSL no garantiza inf/NaN para `b == 0`; el test de 2.0c.3c debe comprobar `1/0`, `-1/0`, `0/0`, `1/-0`, `-1/-0`. Si el driver devuelve `0` o un finito, el test falla con mensaje explícito; no se enmascara. Si el driver no cumple, 2.0c.4 decide si se añade `SignedZeroInfNanPreserve` o se cambia de backend.
- Vectores: `1/0=+inf`, `-1/0=-inf`, `0/0=NaN`, `1/-0=-inf`, `-1/-0=+inf`, `1/2=0.5`, `-1/2=-0.5`.

**Alcance de 2.0c.3b (consecuencia):**
- Implementar los 12 opcodes nuevos (`step, smoothstep, exp, log, log2, log10, acos, asin, atan, atan2, sign, fract`) en los 5 consumidores: `bytecode_vm.h`, `ray_march_simd.h`, `glsl_gen.cpp`, `spirv_gen.cpp`, `jit_compiler.cpp`.
- Corregir `mod` y división en los consumidores CPU (arriba). El path `BcOp` antiguo (`bytecode_vm.h:148-149`) tiene los mismos fallos; se corrige por consistencia, no lo usa el `.ont` v1/v2.
- Incluir el **compilador de expresiones de parámetro SDF a bytecode** en `herm_ont.cpp` (hoy `sdfPrimToBytecode` rechaza `is_expr` por 2.0c.1). Sin él, 2.0c.3c no puede compilar una escena que use las funciones y el criterio "todas las que compilan" no se puede medir. Si el usuario quiere separarlo, es un sub-paso adicional antes de 2.0c.3c.
- No tocar `--render` (DL-32 va después). No tocar los operadores `smooth_union`/`displace` (DL-32). No cambiar la cabecera `.ont` (DL-34 ya fijó v1/v2).
- `tanh`: sin opcode en los 12; si se quiere en el test, se necesita un 13º opcode y una decisión explícita. No se añade por cuenta del agente.

**Orden y criterio de aceptación de 2.0c.3a:**
- Este commit contiene solo documentación: `DECISION_LOG.md` (autoritativo y copias) y `docs/08-changelog.md`.
- No se modifica código, tests ni assets. `git status` de ambos repos solo muestra estos archivos (más el borrado preexistente `docs/21-MAPA_REAL_MOTOR.md` del motor, ajeno a este paso).
- 2.0c.3b no empieza hasta que este commit exista.
- 2.0c.3c escribe `tests/test_eval_parity.cpp` con la tabla de arriba. 2.0c.3d ejecuta, corrige y documenta. Cualquier cambio de tolerancia exige medición y una entrada nueva en este log; no se ensancha en silencio.
- DL-32 (fallo ruidoso de `--render`, 106 parámetros) va después de 2.0c.3. 2.0c.4 solo si la división GPU no cumple IEEE.

**Evidencia de la verificación (2026-11-08):**
- `herm/herm_eval.cpp:88` (división IEEE), `:208-209` (`mod` GLSL), `:225-228` (`smoothstep`).
- `render/bytecode_vm.h:61-62` (máscaras/fmod), `:148-149` (BcOp antiguo).
- `render/ray_march_simd.h:95-107` (máscaras/fmod).
- `render/jit_compiler.cpp:29` (fmodf), `:173-188` (máscara división), `:287-307` (mod).
- `render/glsl_gen.cpp:326-327`; `render/spirv_gen.cpp:66-67`.
- `herm/herm_render.cpp:152-160`, `:202`, `:220` (`--render`).
- `DECISION_LOG.md:611-631` (DL-32: medición de `--render`, `plane`/`capsule`/`cone`).

**Medición real posterior (2026-11-10, RTX 3060 Laptop, driver NVIDIA 616.92):**
- Probe `tests/test_ulp_probe.cpp` (motor) con 395 valores por función; dos ejecuciones idénticas. Detalle en `motor-grafico/docs/22-medicion-ulp-gpu.md` y salida cruda en `motor-grafico/docs/measurements/ulp_rtx3060_2026-11-10.txt`.
- Rango normal IEEE, mín/mediana/máx ULP: `exp` 0/0/5; `log` 0/1/24 321 578; `log2` 0/1/23 947 835; `log10` 0/0/23 699 219; `acos` 0/0/1; `asin` 0/0/2; `atan` 0/0/1; `atan2` 0/0/2; `tanh` 0/0/2.
- Los máximos de `log*` son la zona `x≈1` (resultado→0); el error absoluto es ≤ 7.7e-6. Fuera de `|x-1| < 1e-3`: `log` 31, `log2` 23, `log10` 27 ULP.
- **FTZ:** NVIDIA flush-ea entradas/salidas denormales en trascendentes (`log(1e-38) = -inf`, `atan(1e-38) = 0`); la CPU x86 las conserva. La paridad bit a bit en todo el dominio no es alcanzable sin igualar el FTZ o excluir denormales.
- **La tabla de 1 ULP de este registro no se cumple en el hardware objetivo.** No se modifica aquí: cualquier tabla nueva, criterio híbrido para `log*` y regla de denormales debe registrarse en **DL-37** antes de escribir `tests/test_eval_parity.cpp`.

---

## DL-36 — 2.0c.3b: compilador de expresiones de parámetro SDF a bytecode y los 12 opcodes.

**Estado:** IMPLEMENTADO (código 2026-11-08; commit 2026-11-10, v0.27). Autorizó la implementación de 2.0c.3b. El registro existió en los tres `DECISION_LOG.md` antes de escribir código.

**Mandato (2026-11-08):** DL-35 fija el alcance: 12 opcodes, compilador de expresiones de parámetro SDF, `mod` GLSL y división IEEE. Orden estricto 2.0c.3a → 2.0c.3b → 2.0c.3c → 2.0c.3d. Este paso **no** toca `--render` (DL-32), **no** toca `smooth_union`/`displace` (DL-32), **no** cambia la sintaxis del lenguaje y **no** añade `tanh`.

**Corrección de numeración (obligatoria):** `ONT_END` vale **34**, no 35. Conteo: `ONT_CONST=0`; 13 variables (1..13); 20 operaciones (14..33); `ONT_END=34`. El comentario de `contrato/ont.h:207` («ONT_END = 35; los nuevos empiezan en 36») y las referencias a «36..47» de DL-34/changelog están desplazados en uno. Los opcodes nuevos son **35..46**, se añaden **después** de `ONT_END` y nunca se reordena 0..34.

| Valor | Opcode | Pila | Semántica |
|---|---|---|---|
| 35 | `ONT_STEP` | pop x, pop edge | `(x < edge) ? 0.0 : 1.0` (GLSL `step(edge,x)`) |
| 36 | `ONT_SMOOTHSTEP` | pop x, pop e1, pop e0 | `t = clamp((x-e0)/(e1-e0),0,1); t*t*(3-2t)` |
| 37 | `ONT_SIGN` | pop x | `(x>0)?1:((x<0)?-1:0)` |
| 38 | `ONT_EXP` | pop x | `exp(x)` |
| 39 | `ONT_LOG` | pop x | `log(x)` |
| 40 | `ONT_LOG2` | pop x | `log2(x)` |
| 41 | `ONT_LOG10` | pop x | `log10(x)` |
| 42 | `ONT_ACOS` | pop x | `acos(x)` |
| 43 | `ONT_ASIN` | pop x | `asin(x)` |
| 44 | `ONT_ATAN` | pop x | `atan(x)` |
| 45 | `ONT_ATAN2` | pop x, pop y | `atan2(y,x)` (GLSL `atan(y,x)`) |
| 46 | `ONT_FRACT` | pop x | `x - floor(x)` |

**Gramática del compilador (idéntica a `ExprParser`):** no se inventa sintaxis; se replica la que ya evalúa `evalExpression`.
- `expr = term (('+'|'-') term)*`
- `term = unary (('*'|'/'|'%') unary)*`
- `unary = ('-'|'+')* power`
- `power = primary ('^' primary)?` — `^` no es asociativo y su lado derecho es `primary`, igual que `herm_eval.cpp:101-112`.
- `primary`: número con notación científica (incluye `.5`, `5.`, `1e-6`, `2^-3`), `(expr)`, constantes `pi`, `e`, `LN2`, `PLANCK`, variables `x`, `y`, `z`, `w`, `t`, y variables de sombreado `nx..lz` (se reconocen para rechazarlas).
- Funciones: `sin cos tan tanh abs sqrt floor ceil exp log log2 log10 acos asin atan sign fract pow min max atan2 mod step clamp lerp mix sample smoothstep`. Las llamadas pueden omitir paréntesis (`sin x`), exactamente como el evaluador; `parseArg` consume `expr()`.
- La salida es postfija, una instrucción de 5 bytes `[u8 op][f32 payload]`, y deja exactamente un valor en la pila.

**Ubicación:** `herm/herm_eval.cpp` y `herm/herm_eval.h`. Se añade `compileOntExpression(const std::string&, OntExprSink&)`; `OntExprSink` expone `emitOp(uint8_t)` y `emitConst(float)`. `herm_ont.cpp` implementa el sink sobre `BytecodeBuilder`. El compilador vive junto al evaluador para que la prueba diferencial `evalExpression` vs bytecode detecte cualquier deriva de gramática. **No** se duplica el parser en el motor ni se guarda un AST en `.rih`.

**Parámetros SDF:** en `sdfPrimToBytecode`, el lambda `cf(i,def)` se sustituye por `emitParam(i,def)`:
- Si el parámetro es constante, se emite `ONT_CONST` con el mismo valor de hoy. Los plegados locales que ya existían (`h/2`, `1.0`, `1.73205`) se conservan para que el bytecode v1 de escenas sin expresiones no cambie.
- Si es expresión, se compila en el punto de uso con `emitParam`. Una expresión usada dos veces (p. ej. `R` en `torus`) se emite dos veces; no hay opcode `DUP` y no se añade.
- Los parámetros que la fórmula `.ont` no usa (`ontParamCount`) y que sean expresión se rechazan: hoy se descartarían en silencio. Los constantes en esos parámetros siguen igual (la divergencia `plane`/`capsule`/`cone` es DL-32).

**Preservación de expresiones hasta `.ont`:**
- `evaluateFrame(const Rih&, float, bool fold_sdf_params = true)`. Las transformaciones se pliegan siempre; el nuevo flag solo afecta a `params[i].is_expr` del árbol SDF.
- Los paths `.ont` de `main.cpp` pasan `false`; `--render`, `--gte` y la emisión de `.rih` siguen con `true`. Así `--render` no cambia y `writeOnt` recibe la expresión viva.
- Una expresión constante (`0.65 + sin(w*2)*0.35`) ya no se pliega para `.ont`: se compila a bytecode. Es el objetivo del paso. `w`/`t` se emiten como `ONT_VAR_W` y el motor los evalúa con `ubo.time`.

**AABB y `d_min` conservadores:**
- Nuevo helper `sdfTreeHasExprParam(nodes, idx)`: recorre el subárbol y devuelve true si alguna primitiva usada tiene `params[i].is_expr`.
- `computeNodeAabbRecursive`: si el subárbol tiene una expresión, el AABB local es `[-1e9,+1e9]` en los tres ejes (y luego se transforma a world como siempre). Una expresión puede mover la superficie a cualquier punto; un AABB finito es mentira.
- `flattenNode`: si el árbol tiene expresión, `fn.d_min = 0` y **no** se llama a `evalSdfTree` para el centro (hoy lee `params[i].constant` y daría un valor espurio).
- `buildBvhRecursive`: si una hoja contiene algún `OntFlatNode` con expresión, `node->d_min = 0` y no se muestrea `evalSdfTree`.
- Se usa `1e9f`, no `inf`, para que el área de superficie del BVH y la aritmética sigan siendo finitas y consistentes con el «miss» existente.

**Validación:** `validateOntCompatibility` deja de muestrear tres puntos (`checkOntSdfParam`) y pasa a ser un **dry-run** de `sdfTreeToBytecode` para el árbol SDF de cada `Node` y cada `def`, descartando el bytecode. Así la validación y la emisión no pueden divergir. `checkOntSdfParam` y `findUnsupportedOntFn` se retiran. El error sigue incluyendo primitiva, índice, expresión y línea.

**Rechazos ruidosos en `.ont`:**

| Caso | Motivo | Acción |
|---|---|---|
| `%` | El evaluador usa `std::fmod`; `ONT_MOD` es GLSL `mod`. Diflieren en negativos y no hay opcode `FMOD`. | Error explícito; sugiere `mod(a,b)`. No hay usos de `%` en `.herm` (medido 2026-11-08). |
| `tanh(...)` | No está entre los 12 opcodes; un 13.º exige decisión explícita. | Error explícito. |
| `sample(...)` | Reentra en el evaluador con un offset; `ONT_SAMPLE` tiene otra semántica y los decompiladores GPU lo convierten en `0.0`. | Error explícito. |
| `nx..lz` | Un parámetro SDF no tiene contexto de sombreado; `glsl_gen` los mapea a `0.0` y `spirv_gen` a vectores indefinidos. | Error explícito. |
| función/variable desconocida | El parser no la conoce. | Error del compilador con posición. |
| `CUSTOM` | Los nombres `terrain`/`rock`/... no son expresiones y el alcance de DL-31 lo rechaza. | Se mantiene el rechazo. |
| expresión en parámetro no usado | El bytecode la descartaría en silencio. | Error explícito. |

**Bug descubierto y corregido: `clamp`.** `herm_eval.cpp:223` calcula `fmax(fmin(hi,lo),x)`, que ignora el límite superior. La documentación (`docs/04-guia-uso.md:299`, `docs/12-guia-sintaxis-completa.md:166`) y `ONT_CLAMP` definen `clamp(x,lo,hi)`. Se corrige a `fmax(fmin(x,hi),lo)`. Vectores: `clamp(1.5,0,1)=1`, `clamp(-0.5,0,1)=0`, `clamp(0.5,0,1)=0.5`. Es un bugfix de semántica, no sintaxis; cualquier escena que dependiera del comportamiento buggy se mide en 2.0c.3d.

**Versión y feature_flags:**
- `ontVersionForBytecode(bc, has_sdf_expr_params, feature_flags)`: `ONT_FEATURE_MATH_EXTENDED` si aparece cualquier opcode 35..46; `ONT_FEATURE_SDF_EXPR_PARAMS` si `has_sdf_expr_params`.
- `version = 2` si cualquiera de los dos flags está; `version = 1` solo si ninguno. Un opcode > 46 es un bug del compilador y lanza.
- `has_sdf_expr_params` se calcula sobre los `OntFlatNode` realmente emitidos (`sdfTreeHasExprParam`), no sobre escenas vacías. Se marca aunque la expresión solo use opcodes v1: el archivo lo produjo el compilador de expresiones.
- Los `.ont` v1 existentes no cambian. Los lectores deben aceptar 35..46 en `ontOpcodeIsKnown`; uno que no lo haga rechaza el archivo con error, no lo ejecuta como 0.

**Motor (cinco consumidores):**
- `render/scene.h`: espejo de los 12 opcodes después de `ONT_END`; `ontOpcodeIsKnown` acepta 0..46; corregir el comentario de numeración.
- `render/bytecode_vm.h` (`execBcRaw`): quitar las máscaras `b == 0` de `ONT_DIV`; `ONT_MOD` a la fórmula GLSL; añadir los 12 opcodes; pila con under/overflow ruidoso; `ONT_END` con pila vacía lanza; `bc_len % 5 != 0` lanza. El path antiguo `BcOp` (`execBc`) recibe la misma corrección de `mod`/división por consistencia, aunque no lo usa el `.ont` v1/v2.
- `render/ray_march_simd.h` (`execBcRaw4`): dejar `_mm_div_ps` sin máscara; `mod` GLSL por carril; 12 opcodes por carril; pila ruidosa; `ONT_END` vacío lanza.
- `render/jit_compiler.cpp`: quitar la máscara de división; `_cdecl_jit_mod_ps_ref` a fórmula GLSL; helpers escalares para los 12; validación de altura de pila en tiempo de compilación (hoy `if (stack.size() >= n)` omite la operación en silencio); `ONT_END` vacío lanza. El JIT es opcional; si el bytecode es inválido, falla ruidosamente, no cae a otra ruta.
- `render/glsl_gen.cpp` y `render/spirv_gen.cpp`: añadir los 12; fórmulas explícitas para `step`, `smoothstep`, `sign`, `fract` y `mod` (`a - b*floor(a/b)`); `log10` como `log2(x) * 0.3010299956639812`; `atan2(a,b)` como `atan(a,b)`; quitar los `abs()` que `spirv_gen` pone alrededor de `pow`/`sqrt`; formato de constantes compartido: `%.9g` con round-trip, `-0.0` explícito, `inf`/`NaN` con `uintBitsToFloat(0x7f800000u/0xff800000u/0x7fc00000u)`; `pop()` vacío, pila final vacía, opcode desconocido y `ONT_SAMPLE` lanzan en vez de devolver `0.0`; las variables de sombreado se emiten como `0.0` en ambos decompiladores (hoy `spirv_gen` emite `n.x`/`v.x`/`l.x` indefinidos) porque el compilador las rechaza y las llamadas CPU pasan 0.
- `render/scene.cpp` (`loadOnt`): ya valida con `ontOpcodeIsKnown`; se añaden pruebas de que 35..46 se aceptan y 47 se rechaza.
- `core/herm_bridge.cpp`: **fuera de alcance**. Su compilador propio sigue plegando expresiones complejas a `0.0` y escribiendo `feature_flags=0`; queda como deuda conocida para DL-32 o un paso posterior. El path de paridad de 2.0c.3 es el `.ont` de línea de comandos.
- `render/ri_optimizer.cpp`: **fuera de alcance**. Solo lo usa `herm_bridge`; pliega división con epsilon y no conoce los opcodes nuevos. Deuda conocida.

**Pruebas de 2.0c.3b (se escriben en este paso):**
- Diferencial: para un corpus de expresiones, `evalExpression(expr,x,y,z,w)` contra la evaluación postfija de `compileOntExpression` con `execBcRaw`. Cubre los 12 opcodes, los vectores de `mod`/división de DL-35, los de `clamp`, constantes, variables, precedencia/asociatividad, llamadas sin paréntesis y notación científica.
- Rechazos: `%`, `tanh`, `sample`, variables de sombreado, función desconocida, `CUSTOM`, expresión en parámetro no usado.
- Primitivas: `sphere(1.0 + x*0.5)`, `torus(1.0 + x, 0.25)`, `box(1.0+w, 1.0, 1.0)` contra la fórmula evaluada en puntos.
- `.ont`: escena con expresión -> `version=2` + `SDF_EXPR_PARAMS`; escena con opcode nuevo -> `MATH_EXTENDED`; escena sin nada -> `version=1`; `loadOnt` acepta y rechaza 47.
- Suites existentes: f0 24/24, f1 10/10, f3 16/16, scene 81/82, os_rhi 30/31; los dos fallos preexistentes (`orbit zoom reduces distance`, os_rhi) se documentan, no se arreglan aquí.

**Fuera de alcance:** `--render` (DL-32), `smooth_union`/`smooth_subtract`/`smooth_intersect`/`repeat`/`twist`/`elongate`/`displace` (DL-32), `tanh` (13.º opcode), `%` (opcode `FMOD`), `sample`, `CUSTOM`, `herm_bridge`, `ri_optimizer`, cualquier sintaxis nueva y cualquier campo nuevo en la cabecera `.ont`.

**Criterio de aceptación de 2.0c.3b:**
- `herm.exe --ont` compila `sphere(1.0 + x*0.5)`, las 12 funciones y los vectores de `mod`/división, y escribe `version=2` con los flags correctos.
- Los 12 opcodes dan el mismo resultado (según la tabla de DL-35) en VM escalar, VM SIMD, JIT, GLSL y SPIR-V.
- `%`, `tanh`, `sample`, variables de sombreado, función desconocida, `CUSTOM` y expresión en parámetro no usado fallan con primitiva, parámetro y línea; no hay fallback silencioso.
- Las suites existentes pasan salvo los dos fallos preexistentes.
- `docs/08-changelog.md` y este registro quedan actualizados con la fecha real y la versión (v0.26).

**Implementación (código 2026-11-08; commit 2026-11-10, v0.27):**
- `contrato/ont.h`: 12 opcodes 35..46, `ONT_LAST_OPCODE`, `ontOpcodeIsKnown`, `ontOpcodeName`; comentario de numeración corregido (`ONT_END = 34`, nuevos 35..46). El orden de pila de `ONT_STEP` queda como pop x (tope), pop edge.
- `herm/herm_eval.h/.cpp`: `OntExprSink` (`emitOp`, `emitConst`) y `compileOntExpression`; replica la gramática de `ExprParser` (incluidas llamadas sin paréntesis, `^` no asociativo, notación científica y constantes). Rechaza `%`, `tanh`, `sample`, variables de sombreado `nx..lz`, funciones/variables desconocidas, con expresión y posición en el mensaje. `clamp` corregido a `fmax(fmin(x,hi),lo)`.
- `herm/herm_ont.cpp`: `BytecodeExprSink`, `emitParam`/`emitParamDiv`; todas las primitivas compilan sus expresiones en el punto de uso; `CUSTOM` y expresiones en parámetros no usados se rechazan; `sdfTreeHasExprParam`; AABB `±1e9`; `d_min=0` en `flattenNode` y hojas BVH; `validateOntCompatibility` es un dry-run de `sdfTreeToBytecode`; `ontVersionForBytecode(bc, has_sdf_expr_params, flags)`.
- `herm/herm_rih.h/.cpp`: `evaluateFrame(..., bool fold_sdf_params = true)`; los paths `.ont` de `main.cpp` pasan `false` y usan un `ont_rih` separado; `--render`/`.rih` conservan `true`.
- `render/scene.h`: espejo de 35..46, `ONT_LAST_OPCODE`, `ontOpcodeIsKnown` acepta 0..46.
- `render/scene.cpp` (`loadOnt`): además de rechazar opcode 47 y flags desconocidos, rechaza un archivo v1 que use opcodes 35..46 («no permitido en v1»); v2 con `MATH_EXTENDED` y opcode 46 se acepta.
- `render/bytecode_vm.h` / `ray_march_simd.h`: division sin máscara (IEEE), `mod` GLSL, 12 opcodes, pila con under/overflow ruidoso, `ONT_END` estricto y bytecode sin `END` lanza; `execBcRaw4` opera por carril para paridad exacta.
- `render/jit_compiler.cpp`: `jitValidateStack` valida altura/opcodes/END antes de generar; máscara de división eliminada; helpers escalares para los 12; `_cdecl_jit_mod_ps_ref` a GLSL `mod`. Un bytecode inválido lanza; no cae al intérprete.
- `render/glsl_gen.cpp` / `spirv_gen.cpp`: 12 opcodes con fórmulas explícitas (`step`, `smoothstep`, `sign`, `fract`, `mod`), `log10` como `log2(x)*0.3010299956639812`, `atan2` como `atan(y,x)`, literales compartidos (`%.9g`, `-0.0`, inf/NaN vía `uintBitsToFloat`), sin `abs()` espurios, pila/END/opcode/SAMPLE ruidosos, variables de sombreado como `0.0`.
- `build_libherm.bat`: se añade `/I "deps\lenguaje-hermetico"` porque `herm_eval.cpp` incluye `contrato/ont.h`.

**Verificación de 2.0c.3b (2026-11-08/10):**
- `tests/test_ont_v2.cpp` + `tests/build_ont_v2_test.bat`: **36/36**. Diferencial de 33 expresiones `evalExpression` vs `compileOntExpression`+`execBcRaw` (bit a bit); `mod` GLSL y división IEEE escalar y SIMD; 12 opcodes en escalar/SIMD/JIT; decompilación GLSL/SPIR-V y compilación con `glslc`; rechazos ruidosos; `loadOnt` v1/v2/47/flags/v1-con-opcode-nuevo.
- Manual `herm.exe --ont`: `sphere(1.0+x*0.5)`, `torus(1.0+z,0.25)`, `box(1.0+w,0.5,2.0-y)` -> `version=2`, `flags=2`, AABB `±1e9`; escena con los 12 -> `version=2`, `flags=3`, opcodes 35..46 presentes; escena plana -> `version=1`, `flags=0`.
- Suites: f0 24/24, f1 10/10, f3 16/16, scene 81/82, os_rhi 30/31. Los dos fallos (`orbit zoom reduces distance`, `bodegon.rih found`) son preexistentes y quedan documentados, no se tocan aquí.

**Deuda que sigue abierta:** `tanh` (13.º opcode), `%` (opcode `FMOD`), `sample`, `CUSTOM`, `herm_bridge` (pliega expresiones complejas a `0.0`) y `ri_optimizer` (no conoce 35..46); todo ello fuera del alcance de 2.0c.3b.

---

## DL-37 — 2.0c.3c: tabla de tolerancias de paridad medida y regla de denormales.

**Estado:** IMPLEMENTADO (2026-11-10, v0.27; test en el motor, sin cambio de versión). Autorizó y cerró 2.0c.3c en lo relativo a opcodes. Sustituye la tabla teórica de DL-35 por la medida en el hardware objetivo; no cambia sintaxis ni código del lenguaje.

**Mandato del usuario (2026-11-10):**
- Acepta la medición ULP de 395 valores por función, dos ejecuciones idénticas, contra doble precisión.
- Antes de registrar DL-37 exige: verificar la fórmula de `exp`, quitar el margen asimétrico, confirmar `mod`/operaciones exactas y definir la tolerancia relativa de `log*`.
- Margen: opción A, sin `+1` en ninguna función, con alcance declarado.
- Denormales: excluirlos del corpus con contador explícito; **no** forzar FTZ/DAZ en la CPU del test.
- `tanh`: fuera de alcance; deuda documentada, sin 13.º opcode por ahora.

**Tabla de tolerancias (fijada):**

| Función | Tolerancia |
|---|---|
| `acos`, `atan` | 1 ULP |
| `asin`, `atan2` | 2 ULP |
| `exp` | 5 ULP |
| `log`, `log2`, `log10` | `abs <= 1e-5` Y `rel <= 1e-5` con suelo absoluto `2e-7` |
| `step`, `smoothstep`, `sign`, `fract` | 0 ULP |
| `mod` | 0 ULP, semántica GLSL `x - y*floor(x/y)` |
| `tanh` | fuera de alcance |
| Denormales | excluidos del corpus, contador explícito |

**Regla exacta de `log*`:**

```
pasa  <=>  |cpu - gpu| <= 1e-5
       Y   |cpu - gpu| <= max(2e-7, 1e-5 * max(|cpu|, |gpu|))
```

Es relativa `1e-5` desde `|resultado| = 2e-2`, con suelo absoluto `2e-7` por debajo. Sin suelo la regla es imposible: `log(0.99999994)` da `d = 5.14e-8` con resultado `8.2e-9` (`rel ≈ 6.3`).

**Corrección obligatoria del borrador:** el borrador pedía `rel <= 1e-5` con «suelo 2e-4». Ese suelo venía de la regla con `rel = 1e-3`, donde `1e-3 * 2e-4 = 2e-7`. Con `rel = 1e-5`, `2e-4` produce `2e-9` y las 3 muestras de cola fallan (verificado: 2 fallos por función). Por eso la regla registrada usa el suelo absoluto `2e-7` explícito, equivalente a un denominador `2e-2`. Con ella: **0 fallos** en las 3 funciones.

**Verificación de `exp`:** `glsl_gen.cpp:399` y `spirv_gen.cpp:139` emiten `exp(x)`; `spirv_gen` genera GLSL y lo compila con `glslc`. El SPIR-V resultante es `%17 = OpExtInst %float %1 Exp %16`, no `Exp2` ni `Pow`. Los 5 ULP son del driver/MUFU; no hay fórmula que corregir.

**`mod` y operaciones exactas:** `mod` ya quedó resuelto en DL-35 y se implementó en 2.0c.3b en las cinco rutas (`bytecode_vm.h:80/197`, `ray_march_simd.h:112-118`, `jit_compiler.cpp:29-31`, `glsl_gen.cpp:349-351`, `spirv_gen.cpp:89-92`); `test_ont_v2` 36/36 cubre los vectores GLSL. `step`, `smoothstep`, `sign` y `fract` usan solo comparaciones, `floor` y aritmética IEEE; 0 ULP. Si `smoothstep` difiere por contracción FMA, se corrige con flags/parentización, no ampliando la tolerancia.

**Medición (2026-11-10, RTX 3060 Laptop, driver NVIDIA 616.92, Vulkan 1.4.351):** 395 valores por función, rango normal IEEE, dos ejecuciones idénticas. Detalle en `motor-grafico/docs/22-medicion-ulp-gpu.md` y salida cruda en `motor-grafico/docs/measurements/ulp_rtx3060_2026-11-10.txt`.

| Función | mín/mediana/máx ULP | máx abs | máx rel |
|---|---|---:|---:|
| `exp` | 0/0/5 | 3.906e-3 | — |
| `acos` | 0/0/1 | 2.384e-7 | — |
| `asin` | 0/0/2 | 2.384e-7 | — |
| `atan` | 0/0/1 | 1.192e-7 | — |
| `atan2` | 0/0/2 | 2.384e-7 | — |
| `log` | 0/1/31 (fuera de `x≈1`) | 7.629e-6 | 2.151e-6 |
| `log2` | 0/1/23 (fuera de `x≈1`) | 7.629e-6 | 2.212e-6 |
| `log10` | 0/0/27 (fuera de `x≈1`) | 3.815e-6 | 2.157e-6 |

En la cola `|resultado| < 1e-4`, el máximo absoluto es `1.073e-7` (`log2`); el suelo `2e-7` deja ~2× de margen. En la banda `|resultado| >= 1e-4`, el máximo relativo es `2.212e-6`; `1e-5` deja ~4.5× de margen.

**Denormales:** NVIDIA hace flush-to-zero de entradas y salidas denormales en las MUFU (`log(1e-38) = -inf`, `atan(1e-38) = 0`), mientras la CPU x86 las conserva. Se excluyen del corpus con contador `subIn`/`subOut`; la CPU de referencia (`execBcRaw`/`execBcRaw4`) sigue sin FTZ. Forzar FTZ en el test crearía una CPU distinta de la del motor.

**Alcance:** RTX 3060 Laptop, driver 616.92, Vulkan 1.4.351. Otras GPUs requieren medición propia. Sin margen asimétrico: ninguna función lleva `+1`.

**Deuda documentada — `tanh`:** existe en `herm_eval.cpp` (CPU) desde 2.0b y funciona en campos escalares; no tiene opcode en el bytecode ONT y 2.0c.1 la rechaza ruidosamente en SDF. No se usa en ninguna librería (solo docs, tests y `herm_eval.cpp`). Se añadirá un 13.º opcode en un paso posterior (2.0c.5 o Fase 3) si alguna librería lo necesita. 2.0c.3c la omite.

**Cobertura de 2.0c.3c (`tests/test_eval_parity.cpp`):**
- Las 12 funciones con opcode: `step`, `smoothstep`, `sign`, `exp`, `log`, `log2`, `log10`, `acos`, `asin`, `atan`, `atan2`, `fract`.
- Vectores por opcode: 5 casos por función (negativos, cero, positivos, grandes, pequeños), más los vectores de `mod` y división de DL-35.
- 52 de las 55 librerías estándar; las 3 con parámetros SDF espaciales quedan fuera y se documentan con su nombre en la salida del test. El barrido se ejecuta en 2.0c.3d.
- Reporte por librería y por opcode; cualquier fallo imprime función, vector, CPU, GPU, ULP y tolerancia. Sin epsilon arbitrario ni fallback silencioso.

**Criterio de aceptación de 2.0c.3c:**
- El test compila, ejecuta y reporta las 12 funciones y los vectores de `mod`/división con la tabla de arriba.
- `exp` se compara con 5 ULP; `log*` con la regla híbrida; exactos con 0 ULP; denormales excluidos y contados.
- Un fallo de tolerancia es fallo ruidoso del test, no un aviso.
- `tanh` no aparece en el test.
- `docs/08-changelog.md` y este registro quedan actualizados con fecha real.

**Resultado (2026-11-10):** `tests/test_eval_parity.cpp` implementado y ejecutado en la RTX 3060 Laptop (driver 616.92, Vulkan 1.4.351). **70/70 casos pasan, 0 fallos, 0 denormales excluidos.** Dos ejecuciones consecutivas idénticas, exit 0. `step`/`smoothstep`/`sign`/`fract`/`mod`/`div` 0 ULP; `exp` máx 4 ULP (tolerancia 5); `log`/`log2` máx 1 ULP; `log10`/`acos`/`asin`/`atan`/`atan2` 0 ULP en los vectores del test. El `maxAbs = 1.486e28` de `exp` corresponde a `exp(80)` (ULP 4, relativo `2.7e-7`) y no es fallo. El manifiesto de librerías queda **PENDIENTE**: las 52 librerías y las 3 con parámetros SDF espaciales se ejecutan y documentan en 2.0c.3d, que también debe reconstruir `herm.exe` (el actual no conoce `--ont`). Detalle por opcode en `motor-grafico/docs/22-medicion-ulp-gpu.md` (sección 10).

**Fuera de alcance:** `tanh` (13.º opcode), `%` (opcode `FMOD`), `sample`, `CUSTOM`, `--render` (DL-32), `herm_bridge`, `ri_optimizer`, cualquier sintaxis nueva y cualquier cambio de cabecera `.ont`.

---

## DL-38 — 2.0c.3d: política IEEE/NaN. Modo A (`SignedZeroInfNanPreserve`) con fallback explícito B.

**Estado:** IMPLEMENTADO (2026-11-12; medición 2026-11-11). Decisión: A (`SignedZeroInfNanPreserve`) con fallback B obligatorio. La consulta y la inyección están cableadas en el motor real; el marcado `bit_exact` del replay/export queda diferido porque todavía no existe replay/export. No cambia sintaxis ni bytecode.

**Implementación (2026-11-12):**
- `render/float_controls.h/.cpp`: `injectSignedZeroInfNanPreserve()` idempotente; añade `OpCapability 4466`, `OpExtension "SPV_KHR_float_controls"` y `OpExecutionMode <entry> 4461 32`. Devuelve false si el SPIR-V es inválido o no tiene `OpEntryPoint`.
- `render/vulkan_core.h/.cpp`: `VulkanContext::queryFloatControls()` consulta `VkPhysicalDeviceFloatControlsProperties` al crear el contexto (después de elegir el `physical_device`) y guarda el resultado en `vk_ctx.szinp_f32`, disponible para replay/export.
- `render/vulkan_pipeline.cpp`: `VulkanSceneData::recreateScenePipeline()` inyecta el modo en todo SPIR-V que llega al pipeline (escena especializada y fallback `ray_march.spv`). Si la inyección falla, lo dice y ese pipeline queda en B; no hay fallo silencioso.
- `visor/visor_app.cpp`: tras `vk_ctx.init`, escribe en `phase6_diag.txt` `[IEEE] modo A activo (SignedZeroInfNanPreserve f32)` o `[IEEE] modo B activo (sin determinismo NaN/inf)`.
- `tests/test_eval_parity.cpp`: usa el mismo `injectSignedZeroInfNanPreserve()` del motor; ya no duplica la inyección. La corrida del corpus con el helper del motor da 5003/5003 filas idénticas al arnés de referencia y 0 fallos de categoría.

**Mandato del usuario (2026-11-11):**
- A si es viable; B si no. Criterio: `>= 15/17` → A; `10–14/17` → zona gris (consultar); `< 10/17` → B.
- El fallback no es degradación silenciosa: consultar la propiedad al arrancar; si `VK_FALSE`, activar B, marcar el replay como no bit-exacto y loguear. Default A si está disponible, B si no, con log explícito del modo activo.
- Sin determinismo bit a bit cross-platform prometido si se usa B.
- No declarar 17/17 hasta que el test cubra reasociación (DL-39).
- Cada exclusión se justifica por propiedad medida, nunca por nombre de función.

**Soporte medido (2026-11-11, RTX 3060 Laptop, NVIDIA 616.92, Vulkan 1.4.351):**

| Propiedad | Valor |
|---|---|
| `shaderSignedZeroInfNanPreserveFloat32` | **VK_TRUE** |
| `shaderSignedZeroInfNanPreserveFloat16/64` | VK_TRUE |
| `shaderDenormPreserveFloat32` / `shaderDenormFlushToZeroFloat32` | VK_FALSE / VK_FALSE |
| `shaderRoundingModeRTE/RTZFloat32` | VK_TRUE / VK_TRUE |
| `denormBehaviorIndependence` | ALL (1) |

A existe en este hardware. Otras GPUs requieren su propia consulta; no se extrapola.

**Inyección validada:** sobre el SPIR-V 1.0 de glslc se añade `OpCapability SignedZeroInfNanPreserve (4466)`, `OpExtension "SPV_KHR_float_controls"` y `OpExecutionMode %main SignedZeroInfNanPreserve 32 (4461)`. `spirv-val` acepta el módulo en `vulkan1.0` y `vulkan1.2`; `spirv-dis` confirma las tres instrucciones. El canario demuestra que el driver no ignora el modo:

| Canario | sin modo | con modo |
|---|---:|---:|
| `0 * (1/0)` | 0 | **NaN** |
| `1 / -0` | -inf | -inf |
| `1 / +0` | inf | inf |
| control `(p.x-p.x)+1` | 1 | 1 |

**Resultado de la medición:**
- Pre-fix de `powParityExpr`: 17 fallos de categoría → 2 con modo (**15/17, 88.2%**, umbral A). Resueltos: `binary_entropy` (3), `entropy3` (3), `abrikosov_vortex` (2), `gibbs_entropy2` (2), `half_life_nth_order`, `pareto_pdf`, `weibull_pdf`, `mutual_info`, `kl_divergence`. Restantes: `std_bridge_statmech_mol[1] translational_partition_function` j=0 y j=4.
- Los 2 restantes **no eran límite de A**: `PLANCK² = 4.39e-67 → 0` en f32, `term = -inf`, y `powParityExpr` devolvía el NaN de `sqrt(-1.0 - abs(A))` en vez de `powf(-inf, 1.5) = +inf`. Corregido en `render/spirv_gen.cpp` y `render/glsl_gen.cpp`: la rama no entera distingue `A < -3.402823466e38` (solo `-inf`) y usa `pow(-A, B)`; conserva `-inf` para exponente entero impar.
- Post-fix, con modo: **0 fallos de categoría en 1042 nodos / 5210 muestras / 55 librerías**; 0 nodos con fallo SIMD. **No se declara 17/17**: el test mide NaN/inf/div0 y SIMD≠escalar, no reasociación. Esa limitación es DL-39.
- Efecto colateral medido: 93/5003 muestras GPU cambian entre sin-modo y con-modo (61 funciones; hasta 98.6% rel en `hash21_y`), por codegen distinto del driver. El corpus debe re-baselinarse con A; no es un fallo del modo.

**Fallback B (política implementada):**
1. `VulkanContext::queryFloatControls()` consulta `VkPhysicalDeviceFloatControlsProperties::shaderSignedZeroInfNanPreserveFloat32` al arrancar.
2. Si `VK_TRUE`: `vk_ctx.szinp_f32 = true`; `recreateScenePipeline` inyecta A en todo SPIR-V; log `[IEEE] modo A activo (SignedZeroInfNanPreserve f32)`.
3. Si `VK_FALSE`: no inyectar; log `[IEEE] modo B activo (sin determinismo NaN/inf)`; `vk_ctx.szinp_f32 = false`. El marcado `bit_exact=false` del replay/export queda diferido hasta que exista replay/export; el modo no se presenta como bit-exacto.
4. Si la consulta falla (sin `physical_device`, sin `vkGetPhysicalDeviceProperties2`): tratar como B y loguear el motivo.
5. Nunca usar A cuando la propiedad sea `VK_FALSE`: `VUID-RuntimeSpirv-shaderSignedZeroInfNanPreserveFloat32-06294` dice "must not be used"; el pipeline puede rechazarse o el comportamiento es indefinido. No hay degradación garantizada.

**Verificación en el motor (2026-11-12):** al cargar `std_astro_cosmology.ont`, `build/phase6_diag.txt` registra `[PHASE6] vk_ctx.init OK`, `[IEEE] modo A activo (SignedZeroInfNanPreserve f32)` y `[VK] Scene-specialized pipeline created` (el driver acepta el SPIR-V inyectado y crea el pipeline). El SPIR-V inyectado por el motor pasa `spirv-val` en `vulkan1.0` y `vulkan1.2`. El corpus con el helper del motor: 0 fallos de categoría, 70/70 opcodes, 5003/5003 filas idénticas entre corridas y contra el arnés de referencia.

**Alcance de A (lo que sí y lo que no):**
- A da semántica IEEE por instrucción para NaN, inf y signo del cero; evita las transformaciones que asumen "sin NaN" (`0*x → 0`, gating con `step`, etc.).
- A **no impide la reasociación**. El spec de SPIR-V lo dice: la preservación de NaN/inf no impide transformaciones que reasocien, aunque desborden a inf/NaN. Por eso A no arregla `magnetic_susceptibility` (rel 1.98) ni garantiza bit-exactitud de valores finitos. Eso es DL-39.
- Tampoco arregla las diferencias de transcendentes (`sin`, `exp`, `pow`) ni la amplificación caótica de `hash*`/`vnoise*`/`fbm*`/`terrain_height`; esas son otra deuda (DL-39 las separa).

**Limitación del test (obligatoria en el cierre):** `test_eval_parity` compara NaN/inf/div0 y SIMD≠escalar, y aplica tolerancias ULP a opcodes. No genera cadenas con underflow/cancelación para forzar reasociación, así que un `PASS` no implica paridad bit a bit de valores finitos. La cobertura de reasociación es DL-39.

**Evidencia cruda:** `motor-grafico/docs/measurements/parity_rtx3060_2026-11-11.txt`; detalle en `motor-grafico/docs/22-medicion-ulp-gpu.md` (sección 11).

**Alcance:** RTX 3060 Laptop, driver 616.92, Vulkan 1.4.351. Sin margen asimétrico. Otras GPUs: consulta propia; A si `VK_TRUE`, B documentado si no.

---

## DL-39 — 2.0c.3d: reasociación de punto flotante. Deuda medida, sin implementación.

**Estado:** MEDIDO (2026-11-11). No implementado. **Prohibido tocar el generador para reasociación hasta que esta deuda se decida e implemente en un paso propio.** La única modificación de esta ronda es el fix de `powParityExpr` (DL-38), que no reasocia nada.

**Mandato del usuario (2026-11-11):** antes de registrar DL-39, medir cuántas de las 55 librerías tienen expresiones donde la reasociación cambia el resultado. Criterio: multiplicaciones/sumas de 3+ términos donde el orden importa (underflow, overflow, cancelación). Reportar librerías afectadas, funciones y ejemplos. Sin ese número no se decide.

**Método:**
- Escaneo estático de las 1042 expresiones GLSL del corpus (`--glsl-out`): parser de la expresión completa, se descarta el wrapper `sphere(abs(fn)+0.5)` y se buscan cadenas maximales de 3+ términos con el mismo operador (`+` puro, `*` puro) y cadenas mixtas (`+/-`, `*//`).
- Medición dinámica: `--precise` sin modo (`NoContraction`) sobre el corpus, comparado con el baseline sin modo (run8). `precise` obliga a no contraer y, en este driver, también impide reasociar; se verificó que emite `OpDecorate NoContraction` (no es un control de reasociación portable).
- Se separa contracción FMA (cambios de ~1 ULP) de reasociación (cambios materiales) y se acota el efecto del wrapper del corpus.

**Resultado estructural (sin wrapper):**
- **55/55 librerías** tienen al menos una función con una cadena de 3+ términos.
- 698/1042 nodos (67%): 477 nodos con suma pura 3+ (52 librerías), 403 con multiplicación pura 3+ (54 librerías); 836 nodos si se cuentan cadenas mixtas.
- El patrón es sistémico por construcción: casi cualquier fórmula física encadena 3+ productos o sumas. La pregunta útil no es "¿hay cadenas?", sino "¿cuántas cambian el resultado?".

**Resultado dinámico (`--precise`, sin modo; run8 vs run10):**
- 119 nodos / 40 librerías cambian al menos una muestra (154/5003 muestras).
- 24 nodos / 13 librerías cambian más de `1e-5`.
- 14 nodos / 4 librerías cambian más de `1e-3`.
- 95 nodos / 37 librerías cambian solo `1e-6..1e-5`: ruido de contracción FMA, no reasociación.

**Desglose de los 24 nodos materiales:**
1. **Reasociación catastrófica demostrada — 1 librería / 1 función:** `std_chem_coordination[8] magnetic_susceptibility`. `C = N_A·m²·μB²/(3k_B)`; `μB² = 8.6e-47 → 0` en f32. La CPU asocia de izquierda a derecha (`N_A` primero) y conserva un término distinto de cero; la GPU asocia `μB·μB` primero, anula `C`, y la salida del nodo pasa de 0.685 (CPU) a 1.42 (GPU) en la muestra 0 (rel 1.98, hasta 2.17e9 ULP). Con `--precise` la GPU coincide con la CPU (≤4 ULP). Es el único caso con firma inequívoca de la función: rel 1.98 en las 5 muestras, muy por encima del ruido del arnés.
2. **Clúster caótico — 3 librerías / 13 nodos, NO es reasociación:** `std_math_core` (`hash12`, `hash21_x`, `hash21_y`, `hash13`), `std_geo_terrain` (`terrain_height`, `ridged_fbm`, `billowy_fbm`), `std_math_noise` (`vnoise2d`, `vnoise3d`, `fbm2d_4`, `fbm3d_4`, `fbm3d_6`, `turbulence3d_4`). Cambian más de `1e-3` con `precise`, pero **`precise` no los arregla**: siguen a rel 0.6 contra la CPU. La causa es la amplificación caótica de diferencias de 1 ULP en `sin`/`floor`/`fract` (`fract(sin(x)*43758.5)`), no el orden de una cadena. Necesitan otra decisión (transcendentes bit-exactas o hash entero), no DL-39. Incluirlos aquí sería un diagnóstico falso.
3. **Ruido del arnés — 10 nodos / 9 librerías, ≤2.2e-5:** `second_order_kinetics_same`, `higgs_mass`, `channel_capacity`, `grating_field`, `open_probability`, `transference_number`, `cylinder_y_field`, `fcc_structure_factor`, `gas_viscosity`, `gas_thermal_conductivity`. `higgs_mass` no tiene ninguna cadena en su función (`1.41421354*(0.7+p.x)`) y aun así muestra el mismo patrón 256 ULP / 1.78e-5 en la muestra 0 que `gas_viscosity`; el wrapper `sqrt(x²+y²+z²)` es una cadena de 3 términos y su cancelación con `abs(fn)+0.5` fija ese suelo. No se atribuyen a la librería sin un corpus sin wrapper.

**Respuesta a la pregunta del usuario:** no son 50 casos sistémicos ni 1 caso aislado; son **1 caso catastrófico demostrado (1/55 librerías)**, con el patrón estructural presente en 55/55 y 13/55 mostrando algún cambio material bajo orden forzado, de los cuales 3/55 son un problema distinto (caos) y 9/55 están en el ruido del arnés. Si el criterio es "cualquier cambio >1e-5 bajo orden forzado", el número es 13/55; si es "reasociación que cambia el resultado de forma atribuible a la función", es 1/55.

**Opciones (ninguna implementada):**
- **A. `precise`/`NoContraction` global en los shaders generados.** Medido: arregla `magnetic_susceptibility` (≤4 ULP) y elimina los 15 fallos de categoría sin el modo DL-38 en este driver. Pero el spec solo garantiza no contraer, no no-reasociar; el efecto observado es del driver, no portable. Coste: inhibe FMA (154 muestras cambian, 95 en el ruido `1e-6..1e-5`) y no arregla hash/noise/terrain. No sustituye a DL-38.
- **B. Reordenamiento seguro en el generador.** Asociación explícita y temporales para cadenas con underflow/overflow; requiere una regla determinista y un test que la mida. Sin `precise`, el driver puede volver a reasociar; con `precise`, se paga A. No implementar hasta decidir.
- **C. Aceptar y documentar.** Solo exclusiones por propiedad medida, nunca por nombre; cada exclusión en este registro. Si el motor no promete bit-exactitud de valores finitos, `magnetic_susceptibility` se documenta como caso conocido y el resto (≤2.2e-5) queda dentro de tolerancia.
- **D. Test de reasociación antes de cualquier 17/17.** Añadir cadenas con underflow/cancelación explícitas (p. ej. `(a*b)*c` vs `a*(b*c)` con `b*c → 0`) y comparar CPU/GPU. Sin ese test, el `PASS` actual no cubre reasociación.

**Recomendación (para decidir, no implementada):** DL-39 no se resuelve con un cambio global de `precise`: arregla 1 caso y no arregla el clúster caótico. La opción de menor riesgo es C + D (documentar el caso medido y añadir el test), y tratar hash/noise/terrain en una deuda aparte. Si el usuario exige bit-exactitud de finitos, hace falta A+B y aun así el clúster caótico queda fuera.

**Cobertura actual:** el test de paridad mide NaN/inf/div0 y SIMD≠escalar; no mide reasociación. Esa limitación queda registrada en DL-38 y es la razón por la que 2.0c.3d no declara 17/17.

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
- **Fase 2:** abierta por mandato del usuario (2026-11-04), en curso. **Paso 2.0a/2.0b:** implementados (v0.21, DL-29). **Paso 2.0c.1:** implementado (v0.22, DL-31). **Paso 2.0c.2:** implementado y validado (v0.23, DL-34). **Paso 2.0c.2b:** implementado y validado (v0.24, DL-33); 0 fallos de notación científica. **Paso 2.0c.3:** siguiente; criterio corregido: "todas las que compilan producen paridad", no 55/55. **DL-32:** medido (0/55 librerías con operadores; 20/49 escenas afectadas por `--render`), orden pendiente de confirmación. **Paso 2.0d:** pendiente de localizar los PNG de referencia. El Paso 2.0 **no está cerrado**.
- **Fases 3-6:** pendientes.
- **Decisiones abiertas:** DL-23, DL-24, DL-25 (Fase 5); DL-28 (`ior`/`transmission` en `material`); DL-32 (fallbacks de operadores SDF y `--render`; medido, orden pendiente); 2.0c.4 (división IEEE vs 0-silencioso).

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
