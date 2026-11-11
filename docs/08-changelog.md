# Changelog

## 2026-11-10 — 2.0c.3c: test de paridad de opcodes CPU/GPU (sin cambio de versión)

### Cambios
- **Nuevo** `tests/test_eval_parity.cpp` + `tests/build_eval_parity.bat` en el motor: compila expresiones a bytecode ONT y compara la CPU (`execBcRaw` escalar y `execBcRaw4` SIMD) contra la GPU real (`SpirvGen::decompileBytecode` → GLSL → `glslc` → SPIR-V → pipeline compute Vulkan), el mismo camino que usa el motor.
- **14 opcodes × 5 vectores = 70 casos**: los 12 de 2.0c.3b (`step`, `smoothstep`, `sign`, `exp`, `log`, `log2`, `log10`, `acos`, `asin`, `atan`, `atan2`, `fract`) más `mod` y división de DL-35. `tanh` no aparece: no tiene opcode.
- **Tolerancias de DL-37 aplicadas tal cual**: 0 ULP para exactos y `mod`/`div`; 1 ULP `acos`/`atan`; 2 ULP `asin`/`atan2`; 5 ULP `exp`; `abs <= 1e-5` Y `d <= max(2e-7, 1e-5*max(|cpu|,|gpu|))` para `log*`. Sin margen asimétrico.
- **Denormales**: se excluyen del corpus con contador explícito `subIn`/`subOut`; la CPU de referencia no activa FTZ/DAZ.
- **Reporte por opcode y por librería**: cada fallo imprime función, vector, CPU, GPU, ULP, `abs`, `rel` y tolerancia; el test no tiene epsilon arbitrario ni fallback silencioso.
- **Manifiesto opcional de librerías** (`--libs`, formato `nombre|expresion|tol`). Sin manifiesto el test reporta **PENDIENTE**, no PASS: el barrido de las 52 librerías que compilan es 2.0c.3d. Las 3 con parámetros SDF espaciales quedan fuera y se documentarán con su nombre en ese paso.

### Verificación (2026-11-10, RTX 3060 Laptop, driver 616.92, Vulkan 1.4.351)
- **70/70 pasan, 0 fallos, 0 denormales excluidos.** Dos ejecuciones consecutivas idénticas, exit 0.
- `step`, `smoothstep`, `sign`, `fract`, `mod`, `div`: **0 ULP**. `mod`/`div` incluyen `mod(1,0)=NaN`, `mod(0,0)=NaN`, `1/0=+inf`, `-1/0=-inf`, `0/0=NaN`, `1/-0=-inf`, `-1/-0=+inf`.
- `exp`: máx **4 ULP** (tolerancia 5). El `maxAbs = 1.486e28` es `exp(80)`, con ULP 4 y relativo `2.7e-7`; no es un fallo.
- `log` y `log2`: máx **1 ULP**; `log10`, `acos`, `asin`, `atan`, `atan2`: **0 ULP** en estos vectores.
- El SIMD coincide bit a bit con el escalar en los carriles comparados.

### Consecuencia
- 2.0c.3c cerrado en lo que respecta a opcodes. **2.0c.3d** queda como el paso que reconstruye `herm.exe` (el actual no conoce `--ont`), genera el manifiesto de las 52 librerías, ejecuta el barrido, corrige y documenta.
- Detalle y tabla por opcode en `motor-grafico/docs/22-medicion-ulp-gpu.md` (sección 10).

## 2026-11-10 — DL-37: tabla de tolerancias de paridad medida (pre-2.0c.3c, sin cambio de versión)

### Decisión
- **Tabla de DL-37 registrada** con la medición real de la RTX 3060 Laptop (driver 616.92, Vulkan 1.4.351): `acos`/`atan` 1 ULP; `asin`/`atan2` 2 ULP; `exp` 5 ULP; `step`/`smoothstep`/`sign`/`fract` 0 ULP; `mod` 0 ULP con semántica GLSL; denormales excluidos con contador explícito. Sin margen asimétrico: ninguna función lleva `+1`.
- **`log`/`log2`/`log10`:** `abs <= 1e-5` **Y** `rel <= 1e-5` con suelo absoluto `2e-7`, es decir `d <= max(2e-7, 1e-5*max(|cpu|,|gpu|))`. El suelo `2e-4` del borrador solo era válido con `rel = 1e-3`; con `rel = 1e-5` produce `2e-9` y falla 2 muestras por función. La regla registrada da **0 fallos** en las 3 funciones.
- **`exp` verificado:** `glsl_gen.cpp:399` y `spirv_gen.cpp:139` emiten `exp(x)`; `glslc` produce `OpExtInst ... Exp`, no `Exp2`/`Pow`. Los 5 ULP son del driver/MUFU; no hay fórmula que corregir.
- **`mod` ya resuelto en DL-35** e implementado en 2.0c.3b en las cinco rutas. `step`/`smoothstep`/`sign`/`fract` son 0 ULP por construcción (comparaciones, `floor`, aritmética IEEE).
- **Denormales:** se excluyen del corpus de paridad con contador `subIn`/`subOut`; no se activa FTZ/DAZ en la CPU de referencia (`execBcRaw`/`execBcRaw4`).
- **`tanh`:** fuera de alcance. Deuda documentada: existe en `herm_eval.cpp` (CPU) desde 2.0b, no tiene opcode ONT, 2.0c.1 la rechaza en SDF y ninguna librería la usa. Se añadirá un 13.º opcode en 2.0c.5 o Fase 3 si hace falta.
- **Alcance:** RTX 3060 Laptop, driver 616.92, Vulkan 1.4.351. Otras GPUs requieren medición propia.

### Consecuencia
- **Desbloquea 2.0c.3c:** `tests/test_eval_parity.cpp` cubrirá las 12 funciones con opcode, 5 vectores por función, los vectores de `mod`/división y las 52 librerías que compilan (3 excluidas por parámetros SDF espaciales). El barrido de librerías se ejecuta en 2.0c.3d.
- Detalle completo en `motor-grafico/docs/22-medicion-ulp-gpu.md` y `DECISION_LOG.md` (DL-37).

## 2026-11-10 — Medición ULP GPU/CPU (pre-2.0c.3c, sin cambio de versión)

### Medición
- **Nuevo** `tests/test_ulp_probe.cpp` + `tests/build_ulp_probe.bat` en el motor: probe Vulkan compute que mide las 9 trascendentes de DL-35 contra la CPU (`expf`, `logf`, `log2f`, `log10f`, `acosf`, `asinf`, `atanf`, `atan2f`, `tanhf`) con **395 valores por función** (negativos, cero, positivos, grandes, pequeños y denormales). Fórmulas GPU idénticas a los decompiladores: `log10 = log2(x)*0.3010299956639812`, `atan2 = atan(y,x)`, `tanh = tanh(x)`.
- **Hardware objetivo:** NVIDIA GeForce RTX 3060 Laptop GPU, driver 616.92, Vulkan 1.4.351. Dos ejecuciones consecutivas dan salida idéntica.
- **Rango normal IEEE** (sin denormales), mín/mediana/máx ULP: `exp` 0/0/**5**; `log` 0/1/**24 321 578**; `log2` 0/1/**23 947 835**; `log10` 0/0/**23 699 219**; `acos` 0/0/**1**; `asin` 0/0/**2**; `atan` 0/0/**1**; `atan2` 0/0/**2**; `tanh` 0/0/**2**. La mediana es 0 en las 9 (1 en `log`/`log2`).
- **`log*` cerca de `x=1`:** el máximo relativo explota porque el resultado tiende a cero; el error absoluto se mantiene ≤ 7.7e-6. Fuera de `|x-1| < 1e-3`: `log` máx 31, `log2` máx 23, `log10` máx 27 ULP.
- **FTZ de denormales:** NVIDIA devuelve `-inf`/`0` para entradas o salidas denormales en trascendentes (p. ej. `log(1e-38) = -inf`, `atan(1e-38) = 0`), mientras la CPU conserva denormales. No es un fallo de categoría en rango normal, pero impide paridad bit a bit en todo el dominio.
- **Documento completo:** `motor-grafico/docs/22-medicion-ulp-gpu.md`; salida cruda en `motor-grafico/docs/measurements/ulp_rtx3060_2026-11-10.txt`.

### Decisión
- **No se cambia la tabla de DL-35 en este paso.** Sigue vigente 1 ULP para las 9 funciones hasta que se registre DL-37 con la tabla medida y la regla de denormales. No se escribe `tests/test_eval_parity.cpp` hasta entonces.

## 2026-11-10 — v0.27 — Paso 2.0c.3b implementado (DL-36)

> Código escrito el 2026-11-08; commit y cierre el 2026-11-10.

### Lenguaje
- **12 opcodes nuevos 35..46** en `contrato/ont.h` (`step`, `smoothstep`, `sign`, `exp`, `log`, `log2`, `log10`, `acos`, `asin`, `atan`, `atan2`, `fract`), añadidos después de `ONT_END = 34`; 0..34 no se reordena.
- **Compilador de expresiones** `compileOntExpression` + `OntExprSink` en `herm/herm_eval.cpp/.h`, réplica de la gramática de `ExprParser`. Rechaza con posición `%`, `tanh`, `sample`, variables de sombreado, función/variable desconocida. `clamp` corregido a `fmax(fmin(x,hi),lo)`.
- **Emisión `.ont`**: todas las primitivas compilan sus parámetros expresión en el punto de uso; `CUSTOM` y expresión en parámetro no usado se rechazan. `validateOntCompatibility` es ahora un dry-run de `sdfTreeToBytecode` (validación y emisión no pueden divergir).
- **AABB/`d_min` conservadores**: subárbol con expresión -> AABB `±1e9`, `d_min = 0`, sin muestrear `evalSdfTree`.
- **`evaluateFrame(..., bool fold_sdf_params = true)`**: los paths `.ont` pasan `false`; `--render` y `.rih` no cambian. `writeOnt` calcula `version=2` + flags (`MATH_EXTENDED`, `SDF_EXPR_PARAMS`) sobre los nodos emitidos. Escena plana sigue produciendo `version=1`, `flags=0`.

### Motor
- `render/scene.h`: espejo de 35..46 y `ontOpcodeIsKnown` (0..46).
- `render/scene.cpp`: `loadOnt` rechaza opcode 47, flags desconocidos y **v1 con opcodes 35..46** («no permitido en v1»). v2 con opcode 46 se acepta.
- `render/bytecode_vm.h` y `render/ray_march_simd.h`: división IEEE sin máscara, `mod` GLSL, 12 opcodes, pila con under/overflow ruidoso, `ONT_END` estricto, bytecode sin `END` lanza.
- `render/jit_compiler.cpp`: `jitValidateStack` pre-valida altura/opcodes/END; máscara de división eliminada; helpers para los 12; `mod` GLSL.
- `render/glsl_gen.cpp` y `render/spirv_gen.cpp`: 12 opcodes con fórmulas explícitas, `log10` como `log2(x)*0.3010299956639812`, literales `%.9g`/`-0.0`/inf/NaN, sin `abs()` espurios, pila/SAMPLE/opcode desconocido lanzan.
- `build_libherm.bat`: `/I "deps\lenguaje-hermetico"` porque `herm_eval.cpp` incluye `contrato/ont.h`.

### Verificación
- **Nuevo** `tests/test_ont_v2.cpp` + `tests/build_ont_v2_test.bat`: **36/36**. Diferencial de 33 expresiones (bit a bit), `mod`/división escalar y SIMD, 12 opcodes escalar/SIMD/JIT, decompiladores GLSL/SPIR-V + `glslc`, rechazos ruidosos y `loadOnt` v1/v2/47/flags.
- Suites existentes: f0 24/24, f1 10/10, f3 16/16, scene 81/82, os_rhi 30/31. Los dos fallos preexistentes (`orbit zoom reduces distance`, `bodegon.rih found`) se documentan, no se arreglan aquí.
- Manual: `sphere(1.0+x*0.5)`, `torus(1.0+z,0.25)`, `box(1.0+w,0.5,2.0-y)` -> `version=2`, `flags=2`, AABB `±1e9`; escena con los 12 -> `version=2`, `flags=3`, opcodes 35..46.

### Deuda abierta
- `tanh` (13.º opcode), `%` (opcode `FMOD`), `sample`, `CUSTOM`, `herm_bridge` y `ri_optimizer`; fuera del alcance de 2.0c.3b.

## 2026-11-08 — v0.26 — Paso 2.0c.3b: registro de decisiones antes de código (DL-36)

### Documentado (sin código todavía)
- **Numeración corregida:** `ONT_END` vale **34**, no 35. Los 12 opcodes nuevos son **35..46**: `step`, `smoothstep`, `sign`, `exp`, `log`, `log2`, `log10`, `acos`, `asin`, `atan`, `atan2`, `fract`. Se añaden después de `ONT_END`; 0..34 no se reordena.
- **Compilador de expresiones:** vivirá en `herm/herm_eval.cpp/.h` (`compileOntExpression` + `OntExprSink`), replicando la gramática de `ExprParser` y con una prueba diferencial contra `evalExpression`. No se duplica el parser en el motor.
- **Preservación:** `evaluateFrame(..., false)` para los paths `.ont`; las transformaciones se siguen plegando. `writeOnt` recibe las expresiones vivas y las compila.
- **AABB/`d_min`:** subárbol con expresión -> AABB `±1e9` y `d_min = 0`; no se muestrea `evalSdfTree`, que lee `params[i].constant`.
- **Rechazos ruidosos en `.ont`:** `%` (fmod vs `mod` GLSL), `tanh` (sin 13.º opcode), `sample`, variables de sombreado `nx..lz`, función/variable desconocida, `CUSTOM` y expresión en parámetro no usado.
- **Bug de `clamp`:** `herm_eval.cpp` calcula `fmax(fmin(hi,lo),x)` e ignora el límite superior. Se corrige a `fmax(fmin(x,hi),lo)`; es la semántica documentada y la de `ONT_CLAMP`.
- **Versión/flags:** `MATH_EXTENDED` si aparece un opcode 35..46; `SDF_EXPR_PARAMS` si algún parámetro emitido es expresión; `version=2` si cualquiera; v1 solo si ninguno.
- **Alcance motor:** los cinco consumidores (VM escalar, VM SIMD, JIT, GLSL, SPIR-V) implementan los 12, corrigen `mod`/división y fallan ruidosamente ante pila inválida u opcode desconocido. `herm_bridge` y `ri_optimizer` quedan fuera y registrados como deuda.

### Estado
- Registro completo en `DECISION_LOG.md` (DL-36). **La implementación no ha empezado**; este commit solo desbloquea 2.0c.3b.

## 2026-11-08 — v0.25 — Paso 2.0c.3a: contrato de paridad CPU/GPU (DL-35)

### Documentado (sin código)
- **Referencia CPU:** `execBcRaw` (`render/bytecode_vm.h:21`) y `execBcRaw4` (`render/ray_march_simd.h:16`), no `--render`. El test compara el mismo bytecode en CPU y en GPU (GLSL/SPIR-V).
- **Tolerancias ULP fijadas:** 0 ULP para `step`, `smoothstep`, `sign`, `mod`, `fract`, `min`, `max`, `clamp`, `abs`, `sqrt`, `floor`, `ceil`; 1 ULP para `exp`, `log`, `log2`, `log10`, `acos`, `asin`, `atan`, `atan2`, `tanh`. La tabla y su justificación están en `DECISION_LOG.md` (DL-35) y se copiarán a `tests/test_eval_parity.cpp` en 2.0c.3c.
- **`mod`:** GLSL `x - y*floor(x/y)`, no `fmod`. El evaluador del lenguaje ya lo hace; la VM de bytecode escalar/SIMD y el JIT usan `fmod`/`fmodf` y devuelven 0 para `y == 0`. 2.0c.3b debe corregirlos.
- **División:** IEEE (`inf`/`NaN`), sin 0-silencioso. La VM escalar/SIMD y el JIT enmascaran `b == 0` a 0; 2.0c.3b debe quitar las máscaras. El test de 2.0c.3c verificará `1/0`, `-1/0`, `0/0`, `1/-0`, `-1/-0`; si el driver no cumple, fallo ruidoso (2.0c.4 decide la salida).
- **Alcance:** 2.0c.3b implementa los 12 opcodes en los 5 consumidores y el compilador de expresiones de parámetro SDF a bytecode. `tanh` no está entre los 12; la fila de la tabla es política y el test la omite salvo que se apruebe un 13º opcode.
- **Orden:** 2.0c.3a → 2.0c.3b → 2.0c.3c → 2.0c.3d; DL-32 después de 2.0c.3; 2.0c.4 solo si la división no se resuelve aquí.

### Verificación
- Solo documentación: no se modificó código, tests ni assets.
- `DECISION_LOG.md` actualizado en el repo del lenguaje, en `repos/DECISION_LOG.md` y en el repo del motor (copia sincronizada; el motor tenía el log parado en DL-30).

## 2026-11-08 — v0.24 — Paso 2.0c.2b: notación científica y constantes que faltaban (DL-33)

### Arreglado
- **`Lexer::read_number`** (`herm/herm_lexer.cpp`) acepta exponente `e`/`E` con signo opcional: `1e-6`, `1E6`, `2.5e+3`. Solo consume la `e` si le siguen dígitos (o signo + dígitos); `1e` y `1e+` siguen fallando de forma ruidosa (`expresion invalida`), no se interpretan como `1` + identificador.
- **El evaluador ya aceptaba exponentes** (`ExprParser::number`, `herm_eval.cpp:241`); el bug estaba solo en el lexer, que partía `1e-6` en tres tokens.

### Añadido
- **Constantes `LN2` y `PLANCK`** en `evalExpression` (`herm_eval.cpp`): `LN2 = 0.69314718055994530942`, `PLANCK = 6.62607015e-34`.
- **Función `tanh`** (una entrada, `std::tanh`), junto a `sin`/`cos`/`tan`.
- **`docs/12-guia-sintaxis-completa.md`**: lista de funciones/constantes actualizada; eliminada la advertencia obsoleta que decía que `step` y `smoothstep` no existían en el evaluador CPU (existen desde 2.0b).

### Medición (2026-11-08, probe P2 sobre 55 librerías / 1070 funciones)
- **Antes (v0.23):** 261/1070 fallan en 32/55 librerías. De ellas, **243 en 29 librerías** eran DL-33 (notación científica).
- **Después:** **45/1070 fallan en 18/55 librerías**. **0 fallos por notación científica.**
- Desglose de los 45 restantes:
  - **18** `expresion SDF invalida`: el probe pasa una función que devuelve vector a un parámetro escalar de `sphere` (limitación del probe, no de las librerías).
  - **14** `no da un numero finito`: los argumentos fijos del probe (`1.2345`) desbordan funciones con constantes físicas (`1/1e-34`, exponenciales grandes).
  - **13** `depende de x/y/z`: parámetros SDF espaciales reales; los resolverá 2.0c.3. Uno de ellos (`bloch_wave`) usa además `exp`, que aún no tiene opcode.

### Verificación
| Caso | Resultado |
|---|---|
| `2.5e+3` como `fov` | 2500.0000 |
| `1E6` como `fov` | 1000000.0000 |
| `5e-1` como `fov` | 0.5000 |
| `1e-6` en `sphere` | compila, exit 0 |
| `1e` / `1e+` | exit 1, `expresion invalida en '1 e'` / `'1 e +'` |
| `LN2 * 2.0` como `intensity` | 1.3863 |
| `tanh(1.0)` como `falloff` | 0.7616 |
| `LN2 * 2.0 + PLANCK * 1e34` | 8.0124 (1.3863 + 6.6261) |

### Pendiente
- **2.0c.3:** opcodes nuevos, expresiones de parámetro SDF y unificación de `plane`/`capsule`/`cone`. Criterio: todas las librerías que compilan dan paridad CPU/GPU.
- **DL-32:** medido; el orden (antes o después de 2.0c.3) queda pendiente de confirmación del usuario.

## 2026-11-07 — v0.23 — Paso 2.0c.2: formato y versionado del `.ont` (DL-34)

### Cambiado
- **`contrato/ont.h`:** `reserved[7] = {}` se inicializa a cero (antes quedaba con basura del stack). `reserved[0]` pasa a ser `feature_flags`; `reserved[1..6]` siguen reservados y deben ser 0.
- **Constantes compartidas:** `ONT_VERSION_V1`, `ONT_VERSION_V2`, `ONT_VERSION_MAX_SUPPORTED`, `ONT_FEATURE_MATH_EXTENDED`, `ONT_FEATURE_SDF_EXPR_PARAMS`, `ONT_FEATURE_KNOWN_MASK`, `ONT_FIRST_NEW_OPCODE` y `ontOpcodeIsV1()`.
- **`herm/herm_ont.cpp`:** `ontVersionForBytecode()` recorre el bytecode en pasos de 5 bytes y devuelve `version = 2` **solo** si aparece un opcode > `ONT_END`. Un `.ont` con solo opcodes v1 sigue saliendo v1. `OntHeader hdr = {}` garantiza `reserved[1..6] = 0`.
- **`herm/herm_ont.cpp`:** `writeOnt` escribe `version` y `reserved[0] = feature_flags` (hoy 0; 2.0c.3 mapeará los flags).

### Motor (`render/`)
- **`scene.h`:** constantes espejo de versión/flags y `ontOpcodeIsKnown()` (hoy solo acepta opcodes v1). `OntScene::load_error` guarda el motivo del rechazo.
- **`scene.cpp`:** `loadOnt` valida magic, versión 1..2, flags conocidos, tamaño exacto del layout, `bytecode_size % 5`, offsets/longitudes de cada nodo y todos los opcodes. Ante cualquier fallo imprime `[ONT] <motivo>` en `stderr` y devuelve `false`.
- **`render.h`:** `Render::loadOnt` ya no cae a `loadDefault` en silencio: registra el rechazo con `std::cerr` y `logDiag`, y devuelve `false`.
- **Los 5 consumidores fallan ruidosamente ante un opcode desconocido:**
  - `bytecode_vm.h` (VM escalar) y `ray_march_simd.h` (VM SIMD): `std::runtime_error`.
  - `glsl_gen.cpp` y `spirv_gen.cpp`: el decompilador lanza; `generate()` devuelve `ok = false` con el nodo y el opcode en `error`.
  - `jit_compiler.cpp`: lanza si el opcode no es v1; devuelve `nullptr` para opcodes v1 que el JIT no implementa (los maneja el intérprete).
- **`core/herm_bridge.cpp`:** el `OntScene` en memoria calcula `version` desde el bytecode y deja `reserved[0] = 0`.

### Aceptación (2026-11-07)
- `.ont` generado por el compilador actual: `version = 1`, `reserved[0..6] = 0`.
- Pruebas del lector: **14/14** — carga v1; rechaza v3, flags desconocidos, v1 con flags, opcode 200 y archivo truncado; acepta v2 con flags conocidos.
- Pruebas de los 5 consumidores: **10/10** — todos fallan con mensaje ante el opcode 200.
- Suites del motor: f0 **24/24**, f1 **10/10**, f3 **16/16**, scene **81/82** (fallo preexistente `orbit zoom reduces distance`), os_rhi **30/31** (fallo preexistente).
- `test_ont_bridge` con `teclado.herm`: OK (43 nodos, 1 BVH, 3 materiales, 310 bytes de bytecode).

### Pendiente
- **2.0c.2b (DL-33):** notación científica en el lexer (`1e-6`), `LN2`, `PLANCK`, `tanh`; 243 funciones / 29 librerías.
- **2.0c.3:** opcodes nuevos, expresiones de parámetro SDF y unificación de `plane`/`capsule`/`cone`. Criterio: todas las librerías que compilan dan paridad CPU/GPU.
- **DL-32:** medido; el orden (antes o después de 2.0c.3) queda pendiente de confirmación del usuario.

## 2026-11-07 — v0.22 — Paso 2.0c.1: el `.ont` deja de mentir con los parámetros SDF (DL-31)

### Cambiado
- **`ExprNode.line` y `Expr.line`** (`herm_ast.h`, `contrato/rih.h`): guardan la línea de origen de la expresión. Solo viven en memoria; no se serializan en `.rih` ni en `.ont`. El layout de `Expr` cambia, así que el motor debe recompilarse con el nuevo contrato.
- **`Parser::parse_expr`** captura la línea inicial; **`Resolver::toExpr`** la propaga; **`sdfPrimToBytecode`** la usa en los mensajes.
- **`validateOntCompatibility(rih)`** (`herm_ont.h/.cpp`): recorre los `sdf_nodes` de nodos y defs. Si un parámetro SDF es una expresión:
  - rechaza las variables por muestra (`nx/ny/nz/vx/vy/vz/lx/ly/lz`);
  - evalúa la expresión en tres puntos distintos de `(x,y,z)`. Si el valor cambia, la declara dependiente del espacio y falla;
  - si el resultado no es finito, falla;
  - si el evaluador lanza, propaga el error con la expresión y la línea.
- **`main.cpp`** llama a `validateOntCompatibility` solo cuando `emit_ont` está activo, **antes** de `evaluateFrame`. En la rama de un solo fotograma, `writeOnt` recibe `evaluateFrame(rih, 0.0f)` para poder plegar expresiones constantes o solo dependientes de `w`.
- **`SdfType::CUSTOM`** se rechaza explícitamente: antes emitía `ONT_CONST 1e9` (un "miss" silencioso). El `default` de tipo desconocido también lanza.
- **`cf()`** (`herm_ont.cpp:47`) deja de devolver el valor por defecto: cualquier `is_expr` que llegue ahí (llamada directa o desde el puente del motor) lanza.

### Aceptación
`sphere(1.0 + exp(x))` con `--ont`:
```
Error: el parametro 0 de 'sphere' depende de x/y/z ('1.0 + exp ( x )') y el
bytecode ONT no puede representarlo; funcion sin opcode: 'exp' (linea 8)
```
exit 1, sin `.ont` ni `.obs`. También fallan `sphere(1.0+x)`, `sphere(1.0+step(0.5,x))` y `custom(y-...)`. Compilan y pliegan: `sphere(1.5*0.5)` → radio 0.75, `sphere(0.65+sin(w*2)*0.35)` → 0.65 en el fotograma 0, `sphere(exp(1.0))` → 2.7183.

### Medición (2026-11-07, 55 librerías, 1070 funciones)
- **P1 estática:** 46/55 usan directamente alguna de las 12 funciones sin opcode; 46/55 también de forma transitiva. Las 9 limpias: `std_astro_cosmology`, `std_astro_mechanics`, `std_astro_stellar`, `std_bio_physiology`, `std_bio_systems`, `std_geo_landscape`, `std_geo_ocean`, `std_phys_kinematics`, `std_phys_relativity`.
- **P2 dinámica:** 261/1070 funciones fallan, en 32/55 librerías. Desglose: **13 funciones en 3 librerías** son parámetros espaciales reales (lo que 2.0c.1 debe rechazar); **243 en 29 librerías** son el bug de notación científica (`1e-6` → `1 e -6`, DL-33), que también falla sin `--ont`; **5 en 4 librerías** dan `inf` con los argumentos del probe.
- **P3 formato:** `sizeof(OntHeader)` real = **120 bytes** (el documento decía 80). `reserved[7]` ocupa los bytes 64–119.

### Consecuencia
2.0c.1 rompe 3/55 librerías, por debajo del umbral de 5 fijado por el usuario. Pero el criterio de 2.0c.3 ("55/55 idénticas CPU/GPU") no es alcanzable todavía: 29/55 fallan antes de emitir bytecode por DL-33, los 6 operadores SDF de DL-32 siguen degradando en silencio en el path `--render`, y `plane`/`capsule`/`cone` no significan lo mismo en bytecode que en CPU.

### Pendiente
- **2.0c.2:** decidir formato (v1→v2 vs `reserved[7]`) antes de tocar opcodes.
- **2.0c.3:** 12 opcodes nuevos + compilar expresiones de parámetro + unificar `plane`/`capsule`/`cone` con el render CPU + actualizar los 5 consumidores.
- **2.0c.4:** división IEEE vs 0-silencioso en GLSL.
- **DL-32/DL-33:** registrar decisión y, si se aprueba, arreglar el lexer antes de 2.0c.3.

### Evidencia adicional (DL-32, 2026-11-07)
`--render` con `sphere(1.5 * 0.5)` y con `sphere(0.75 + x)` da el **mismo PNG de 257 B** (hash idéntico) y exit 0; `sphere(0.75)` da 862 B. Cualquier expresión de parámetro se renderiza como `sphere(0)` sin avisar.

## 2026-11-04 — v0.21 — Paso 2.0a + 2.0b: `evalExpr` evalúa de verdad y las 13 funciones existen (Paso 2.0 **no cerrado**: faltan 2.0c y 2.0d)

### Arreglado (2.0a, DL-29)
- **`Resolver::evalExpr`** (`herm_resolver.cpp:83`): ya no usa solo `std::stof`. Si el texto no es un número puro, evalúa con `evalExpression` (el mismo evaluador del runtime). Antes devolvía `0.0f` en silencio para cualquier expresión.
- **Guardas nuevas:** si el valor cambia al variar `(x,y,z,w)` → `ResolverError` ("depende de x/y/z/w y se usa en un campo escalar constante"). Si el resultado es `inf`/`nan` → error (el `.rih` es JSON y no admite esos literales).
- Campos que ahora se evalúan: `axes.X/Y`, `fov`, `aperture`, `focal_dist`, `density_scale`, `intensity`, `falloff`, `roughness`, `metallic`, `opacity`, `k`/`spacing`. Antes, cualquier expresión daba 0.
- Verificado: `density_scale: 1.0 + 1.0` → `2.0000`; `sqrt(4.0)` → `2.0000`; `exp(1.0)` → `2.7183`; `step(0.5, 0.7)` → `1.0000`.

### Añadido (2.0b)
- **13 funciones** en `herm_eval.cpp`: `step`, `smoothstep`, `exp`, `log`, `log2`, `log10`, `acos`, `asin`, `atan`, `atan2`, `sign`, `mod`, `fract`. Semántica GLSL: `mod` usa `x - y*floor(x/y)` (no `fmod`), `step(edge,x)` devuelve 0 si `x < edge`.
- **Sin fallback silencioso:** función o variable desconocida, paréntesis sin cerrar, `)` sin abrir, carácter inesperado, `sample` anidado y expresión inválida lanzan `std::runtime_error` con el texto de la expresión. Se eliminaron los `catch(...) { return 0.0f; }` de `evalExpression`.
- **División IEEE:** `/` ya no devuelve 0 cuando el divisor es 0; produce `inf`/`nan` como la GPU (necesario para la paridad). El `.rih` lo rechaza en campos escalares.
- **`docs/12-guia-sintaxis-completa.md` §5.4** corregido: afirmaba que el path GPU soportaba todas las funciones GLSL. No es cierto — el bytecode ONT no tiene esos opcodes.
- **`libreria/tests/test_paso2_funciones.herm`**: test permanente. Escena que usa las 13 funciones en campos escalares; el `.rih` debe dar `fov=32`, `density_scale=14.2661`, `intensity=1.5`, `falloff=0.25`, `tensor[7]=1.0`.

### Verificación (16 expresiones + 5 fallos)
| Expresión | Emitido |
|---|---|
| `1.0 + 1.0` | 2.0000 |
| `sqrt(4.0)` | 2.0000 |
| `exp(1.0)` | 2.7183 |
| `smoothstep(0.0, 1.0, 0.25)` | 0.1562 |
| `log(e)` / `log2(8)` / `log10(1000)` | 1.0000 / 3.0000 / 3.0000 |
| `acos(0.5)` / `asin(0.5)` / `atan(1.0)` / `atan2(1,1)` | 1.0472 / 0.5236 / 0.7854 / 0.7854 |
| `sign(-3.5)` / `mod(-1.5,1)` / `fract(2.75)` | -1.0000 / 0.5000 / 0.7500 |
| `max(0, 1 - abs(0.5-0.25)) * 4.0` | 3.0000 |

Fallos ruidosos verificados (exit 1): `foo(1.0)`, `sqrt(4.0`, `w * 2.0`, `x + 1.0`, `1.0 +`, `sample` anidado, `1.0 / 0.0`.

### Criterio 2.0a cumplido
- `ejemplos/teclado.herm`: **33 fotogramas**, cámara `[0, 4.5, 9]` fov 32, materiales `cobre [0.95,0.64,0.38]`, `plateado [0.85,0.85,0.88]`, `plastico_negro [0.08,0.08,0.10]`, y la animación varía: la tecla 1 pasa de `y=0.2000` (frame 0) a `y=0.0800` (frame 30).
- 55/55 librerías siguen compilando. 12/13 ejemplos compilan; `bodegon.herm` sigue fallando por `ior` (DL-28).

### Pendiente (bloquea el cierre del Paso 2.0)
- **2.0c — paridad CPU/GPU:** el bytecode ONT no tiene opcodes para 12 de las 13 funciones nuevas, y `ONT_DIV`/`ONT_MOD` devuelven 0 cuando el divisor es 0. Extender `OntOpcode` + sus 5 consumidores (VM, VM SIMD, GLSL, SPIR-V, JIT) es trabajo previo al test de paridad.
- **2.0d — re-ejecutar benchmarks `--render`:** no se localizaron los PNG previos en los repos; hay que fijar dónde viven o regenerarlos.

## 2026-11-04 — v0.20 — Cierre de la reescritura: 55/55 librerías compilan + `materiales.hm` archivado (DL-27)

### Resultado
- **55/55 librerías estándar compilan** con `build/herm.exe` (DL-14 + DL-26). Barrido completo en ~1 s, exit code 0 en todas. Antes: 35/55.
- **20 librerías reescritas a mano** sin tocar el lenguaje (ni `if`, ni `for`, ni comparadores, ni estado): 21 `.hm` modificados, −4823/+3892 líneas.
  - `for` → `def`/`instance` repetidos: `std_bridge_quantchem`, `std_chem_theoretical`.
  - `if`/`else` → `mix`/`step`/`smoothstep`: `std_bridge_biophysics`, `std_bridge_enzymology`, `std_chem_coordination`.
  - Keyword como identificador → renombrado: `std_bridge_biochem`, `electrochem`, `genomics`, `statmech_mol`, `chembio`, `physchem`, `chem_optics`, `bio_development`.
  - `let` en cuerpo de `node` → movidos a `fn` o top-level: `std_bridge_atomic`, `molbio`, `spectral`.
  - Otros: `std_bio_genetics` (coma espuria), `std_bridge_kinetics` (`q_ratio`), `std_chem_inorganic`/`std_chem_supramolecular` (llaves desbalanceadas).

### Añadido
- **`docs/12-guia-sintaxis-completa.md`**: guía de sintaxis y capacidades reales del lenguaje (pre-Fase 2), para que nadie escriba construcciones que el parser rechaza.
- **`docs/librerias/*.md`** (20): documentación actualizada de cada librería reescrita — qué se cambió y por qué.

### Archivado (DL-27)
- **`libreria/materiales.hm` → `archived/libreria/materiales.hm`**: único `.hm` que no compilaba. Catálogo pre-tensor que usa `ior`/`transmission`, campos que `parse_material` no acepta. No se convierte al tensor (el mapeo no está definido) ni se borra (se conservan los valores de referencia).
- **`ejemplos/teclado.herm`**: quitado el `import "materiales.hm"` y añadida la definición local de `cobre` (único material que usaba del catálogo). Vuelve a compilar: 33 fotogramas, 3 materiales, 40 nodos.

### Corregido (documentación)
- **`libreria/README.md`**: estado real 55/55 con historial (35/55 el 2026-10-16), sección de las 20 reescritas y nota de `materiales.hm` archivado.
- **`DECISION_LOG.md`**: "Estado actual" actualizado (Fase 0 y 1 cerradas, Fase 2 no se abre sin aprobación explícita). Antes decía "Fase 0 lista para ejecutar / Fase 1 pendiente", falso desde el 2026-10-16.
- **Referencias al catálogo**: `README.md`, `docs/04-guia-uso.md` (ejemplo de import ahora `std_math_core.hm`), `docs/contrato-ontologico.md`.

### Pendiente conocido (no se toca aquí)
- **`ejemplos/bodegon.herm`** falla por `ior` en un `material` inline (L29). Misma carencia del parser que DL-27 documenta; soportar `ior`/`transmission` es una decisión de lenguaje que corresponde al usuario.
- **`herm_eval.cpp`**: 13 funciones sin implementar y fallback silencioso `return 0.0f` → Paso 2.0, siguiente trabajo de Fase 2.
- **RAM bomb** del resolver (1568 MB en `v_lets.herm`) → DL-09.

## 2026-10-23 — v0.19 — Fix `substExprText`: `_` como carácter de palabra (DL-26) + entrada DL-22..26 en DECISION_LOG

### Arreglado
- **`substExprText`** (`herm_resolver.cpp:507`): el límite de palabra era `std::isalnum`, que **no incluye `_`**. Identificadores con `_` se corrompían al sustituir parámetros de `def`/`fn`: `substExprText("ancho_2 + ancho", {"ancho":"0.5"})` producía `"0.5_2 + 0.5"` (silencioso). Ahora: `isalnum(c) || c == '_'` — criterio estándar C/C++/Rust/Python. `"ancho_2 + 0.5"` correcto. Test mínimo: los 4 casos de `DL-26` en el DECISION_LOG.

### Añadido (DECISION_LOG, ambos repos)
- **DL-22** — Intersección analítica para composiciones de primitivas puras (Paso 5.14 en PLAN.md, sub-pasos 5.14.1–5.14.6, depende de Fase 3 + DL-09).
- **DL-23** — Política de decisión de pipeline (SDF vs rasterización). PENDIENTE.
- **DL-24** — Compositing unificado (depth buffer + UBO compartido). PENDIENTE.
- **DL-25** — Sistema de shading unificado (mismo PBR, luces, sombras). PENDIENTE.
- **DL-26** — Fix `substExprText` `_`. Registrado con tests mínimos + bug latente `unordered_map` (orden de sustitución no determinista, pendiente).
- **Paso 2.0** en PLAN.md — Funciones faltantes en `herm_eval.cpp` (13 funciones + fallback a error). PRIORIDAD ALTA antes de DL-09: incorrecto > lento.

### Nota (patrón del proyecto)
Tres fallos silenciosos encontrados en una semana: funciones faltantes en `evalExpression` (devuelve 0.0f), `substExprText` con `isalnum` sin `_` (corrompe identificadores), benchmark RAM (peak=0 por muestreo mal). Disciplina: **todo fallo debe ser ruidoso** — si algo no se soporta, error; si algo no existe, error; si algo diverge, error.

## 2026-10-16 — v0.18 — DL-14 validado con edge cases + fixes de shadowing/sin-retorno + regresión verde

### Arreglado (DL-14, encontrados por edge-case testing)
- **Shadowing invertido** (`herm_resolver.cpp`): el cuerpo del `fn` sustituía params ANTES que lets — `fn "sh"(x){ let x=1; let x=2; x }` con `sh(9)` devolvía 9 (el param) en vez de 2 (el último let). Ahora el cuerpo aplica `let_subst` primero (scope interno gana) y `param_subst` después. Los lets siguen viendo params/lets-antiores en sus valores (el inicializador de un let no se ve a sí mismo).
- **`fn` sin expresión final** (`herm_parser.cpp`): `fn "f"(x){ let y=x; }` aceptaba body vacío y emitía `()` silencioso. Ahora: error `fn 'f' debe terminar con una expresion de retorno`.

### Añadido
- **`libreria/tests/test_dl14_fn_lets.herm`**: test permanente DL-14 — lets encadenados, params en lets, shadowing, combinación a+b. Verificado: chain(5)=9, double_plus(3)=7, shadow(9)=2, mix(2,3)=23.

### Edge cases DL-14 (verificación completa)
| Caso | Resultado |
|------|-----------|
| lets encadenados (`let a=x+1; let b=a*2; let c=b-3; c`) | ✔ en orden secuencial |
| let referencia param (`let y=x*2; y+1` con x=3) | ✔ 7 — param dentro del valor del let |
| shadowing (`let x=1; let x=2; x` con x=9) | ✔ 2 — gana el último let (tras fix) |
| sin expresión final (`fn{ let y=x; }`) | ✔ error explícito (tras fix) |
| referencia circular (`let a=b; let b=a`) | limitación conocida: no cuelga (depth-limit 16), filtra `b` como identificador libre — comportamiento preexistente del resolver para nombres indefinidos, no regresión |
| valor de let referencia otro let + param | ✔ mix(2,3)=23 |

### "let en nodo" (diagnóstico pedido)
**No es bug ni feature pendiente — es sintaxis no contemplada.** La BNF (`docs/03` §4.1: `node_block ::= "node" STRING? "{" node_body "}"`) no incluye `let` en `node_body`; `parse_node_body` no tiene case LET → error `inesperado en nodo: 'let'` correcto y preexistente (no tocado por DL-14). Las 3 libs (`std_bridge_atomic`, `molbio`, `spectral`) la usan ilegalmente → al reescribir, los lets se mueven a top-level o `fn`.

### Regresión (tras fixes de shadowing/sin-retorno)
- **55 librerías: 35 PASS / 20 FAIL** — mismo set exacto que pre-fixes, sin ninguna lib nueva rota.
- **Motor: F0 24/24, F1 10/10, F3 16/16** — `libherm.lib` reconstruido con los fixes; `compileToRih` intacto (el camino sin fn — lets de escena — no afectado; sustitución de lets de escena va por `let_table` aparte, sin cambios).

## 2026-10-16 — v0.17 — DL-14: `let` internos en cuerpo de `fn` + medición de RAM arreglada

### Añadido
- **DL-14 (parser):** el cuerpo de `fn` ahora acepta `let` internos antes de la expresión final. `AstFn.body_lets` (AST), bucle de `let`s en `parse_fn` (parser), desazúcar a sustitución textual en `expandExpression` (resolver). La última expresión del cuerpo es el valor de retorno. Sin `if`/`for` internos (DL-13/DR-03 vigentes).
- **Verificado:** `fn "area"(b,h){ let b2=b*b; let h2=h*h; sqrt(b2+h2) }` → `.rih` con `sqrt(((3)*(3))+((4)*(4)))`, sin nombres fugados.

### Arreglado (harness de tests, no forma parte de la repo)
- **Medición de RAM:** el poll anterior dormía 200 ms ANTES de muestrear y confiaba en `$p.HasExited/Handle/ExitCode` — herm.exe tarda ~17-50 ms, así que el proceso moría antes del primer sample (`peak=0` siempre) y el objeto de PowerShell llegaba pos-mortem con `Handle`/`ExitCode` nulos (bug clásico de `Start-Process -PassThru`). Ahora: muestreo por pid con `Get-Process` a los 5 ms desde t=0, status por existencia del `.rih` + contenido de stdout/stderr (sin `ExitCode`), borrado de salidas previas antes de cada lanzo.
- **Tiempos reales:** antes ~208 ms = sleep; ahora 17-50 ms de herm real.

### Resultado (55 librerías, tras DL-14)
- **35 PASS / 20 COMPILE_ERR** (mismo conteo que sin DL-14 — los 20 fallan ahora por `for`/`if`/keywords/`let` en nodos, es decir, por lo que DL-13/14 dicen resolver **reescribiendo las librerías a mano**; ahí se detiene la automatización por decisión del usuario).
- **Test combinado (6 libs + 31 let):** `v_lets` = **RAM_BOMB, 12,973 ms, peak 1568 MB** — sigue pendiente DL-09 (fold+memo+CSE, objetivo < 100 MB).

## 2026-10-16 — v0.16 — Decisiones DL-09/13/13b/14/20/21: el lenguaje no cambia

### Decidido (por el usuario, registrado en DECISION_LOG)
- **DL-13 RECHAZADO** — no hay `if`. Ni como expresión ni como statement.
- **DL-13b RECHAZADO** — no hay comparadores (`<`, `>`, `==`, `&&`, `||`) como sintaxis. Las comparaciones se hacen con `step`/`smoothstep`/`abs`/`clamp`/`min`/`max`.
- **DL-14 DECIDIDO (opción B)** — el parser acepta `let` internos en el cuerpo de `fn`; la última expresión es el retorno. Sin `for` interno, sin `if` interno. Las 17 librerías caídas se reescriben a mano con las funciones existentes.
- **DL-09 DECIDIDO (A+B+C en orden)** — constant folding → memoización de `fn` → CSE. Criterio de aceptación: `test_layer1_math` (6 libs + 31 let) corre en < 100 MB.
- **DL-20 DECIDIDO** — el editor de nodos (visual, DAG) es la capa de comodidad; traduce a `.herm` puro. No se toca el lenguaje. Sub-decisiones DL-20.a/b/c pendientes.
- **DL-21 PENDIENTE** — comprensiones declarativas (hoy: reescribir a mano; recom. posponer a Fase 5+).

### Consecuencia para el lenguaje
**Cero cambios de sintaxis.** El lenguaje queda exactamente como está (~20 tokens). La única extensión es del parser (let internos en fn, DL-14).

## 2026-10-16 — v0.15 — Fase 0 (asegurar): pipeline ONT commiteado + 55 librerías migradas

### Recuperado (commit de emergencia, trabajo sin versionar desde julio)
- **Pipeline `.ont`/`.obs` completo** que nunca se había commiteado: `herm_ont.cpp` (1,211 LOC), `herm_obs.cpp`, `herm_compile.cpp`, `herm_ont.h`, `herm_obs.h`, `contrato/ont.h`.
- **Fixes acumulados sin commitear** en `herm_parser.cpp`, `herm_resolver.cpp`, `herm_lexer.*`, `herm_render.cpp`, `herm_rih.cpp`, `herm_ast.h`, `main.cpp`, `contrato/rih.h`, `operador_tb/renderer.cpp`.
- **`herm/build.bat`**: añade `herm_ont.cpp` y `herm_obs.cpp` al SRC del CLI.

### Añadido
- **`libreria/`**: migradas las **55 librerías estándar** (`std_math_*`, `std_phys_*`, `std_bridge_*`, `std_chem_*`, `std_bio_*`, `std_geo_*`, `std_astro_*`) desde `proyecto de IA artistica/Lenguaje Hermetico/.gitignored/libreria/` (copia local sin git, creada 2026-07-13).
- **`libreria/tests/`**: 44 `.herm` de prueba + `verify_optics.hm` migrados del mismo origen.
- **`docs/librerias/`**: 56 documentos (índice + uno por librería).
- **`docs/investigacion/`**: 16 archivos de investigación/auditoría (`kiro_librerias_investigacion`).
- **`libreria/README.md`**: estado del test de las 55 librerías (resultado: **35/55 compilan, 20 fallan** por `fn` multi-statement (17) y keywords como parámetros (3); RAM bomb de 1.5 GB en `test_layer1_math`).
- **`DECISION_LOG.md`** y **`PLAN.md`** en la raíz (plan de reforma de 6 fases; regla: el agente no decide arquitectura sin consultar al usuario).

### Nota
- El changelog no registraba nada desde v0.14 (2026-06-04): todo el trabajo de julio–octubre (pipeline ONT, `.obs`, imports) queda registrado ahora, tarde pero registrado.
- Tag `pre-reforma` apunta a este commit.

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
