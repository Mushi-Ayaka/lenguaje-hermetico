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

### DL-09 — Cómo resolver la RAM bomb del resolver.

**Estado:** PENDIENTE
**Contexto:** 31 `let` con funciones de 6 librerías → 1.5 GB RAM. Causa: sustitución textual sin memoización ni CSE.

**Opciones:**
- **A) Memoización de `fn`.** Cache de `(fn_name, arg_text)` → `expanded_text`.
- **B) CSE.** Detección de subexpresiones idénticas en el árbol expandido.
- **C) Constant folding real.** Evaluar en compilación cuando todos los argumentos son constantes.
- **D) Límite de profundidad más agresivo.** No arregla el problema, solo lo retrasa.

**Recomendación:** A + B + C. Los tres son ortogonales y los tres son necesarios para que el resolver escale.

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

### DL-13 — Ruta del lenguaje a un `if` controlado.

**Estado:** PENDIENTE
**Contexto:** El lenguaje no tiene control de flujo. Se evalúo si añadir `if` para condicionales. La pureza lo permite si es `if` como expresión, no como statement.

**Opciones:**
- **A) No añadir.** Todo se hace con `min`/`max`/`clamp`/`step`/`smoothstep`.
- **B) Añadir `if` como expresión.** `if cond then a else b` → `mix(a, b, cond)`. Sigue siendo puro.
- **C) Añadir `match` para casos múltiples.** Más general.

**Recomendación:** B. `if` como expresión puro no rompe nada y hace las librerías más legibles.

### DL-14 — `fn` con cuerpo multi-statement.

**Estado:** PENDIENTE
**Contexto:** 17 librerías fallan porque el parser solo acepta una expresión por cuerpo de `fn`.

**Opciones:**
- **A) Extender el parser.** Permitir `let` internos. El último statement es el valor de retorno.
- **B) Reescribir las 17 librerías.** Convertir cada una a una sola expresión.

**Recomendación:** A. Reescribir 17 librerías es trabajo manual, y la extensión del parser es pequeña.

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

**Criterio de aceptación de Fase 5:**
- `teclado.herm` con 38 teclas renderiza con 1 SDF en el shader.
- Escenas con `displace` funcionan.
- Benchmarks de rendimiento mejoran respecto a Fase 3.

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

- **Fase 0:** lista para ejecutar.
- **Fase 1:** pendiente de las decisiones DL-07 a DL-14.
- **Fase 2:** pendiente de Fase 1.
- **Fases 3–6:** pendientes.

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
