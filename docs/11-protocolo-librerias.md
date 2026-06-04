# Protocolo de Generación de Librerías Herméticas (IA)

Este documento es un **manifiesto y conjunto de instrucciones** diseñado para ser leído por agentes de Inteligencia Artificial que sean asignados para expandir el "Lenguaje Hermético". Su propósito es guiar la investigación y la transcripción de ciencias reales (física, química, biología) al plano matemático tensorial `[x, y, z, w, r, g, b, a]`.

---

## 1. El Paradigma Hermético y la Macro-Expansión

Los agentes deben comprender que el Lenguaje Hermético **no es un motor de físicas ni de renderizado tradicional**. No existe estado que persista frame a frame, no existen polígonos, ni un bucle de simulación mutable.

El universo Hermético es un bloque sólido tetradimensional. El trabajo del agente es investigar heurísticas, ecuaciones y principios del mundo real y destilarlos a **funciones matemáticas puras** mediante el uso del keyword `fn` y `def`.
La directiva `fn` permite construir **macros textuales recursivas**. Las librerías de ciencias enteras escritas por IA son aplanadas por el compilador Hermético en una única, inmensa ecuación evaluable en O(1) durante el render. Al basarse enteramente en las matemáticas subyacentes, no hay impacto por anidar una ciencia sobre otra.

---

## 2. El Árbol Ontológico (La Jerarquía de la Ciencia)

Más que una pirámide lineal, la construcción del conocimiento en el Lenguaje Hermético se comporta como un **árbol ramificado**. La física aporta las raíces y las leyes fundamentales, y a medida que subimos, la complejidad aumenta, ramificándose en la química y floreciendo en la biología.

Para "pintar" la realidad con una fórmula matemática que se hereda y se complejiza desde la base hasta la copa, los agentes deberán adherirse al siguiente esquema jerárquico de bibliotecas `.hm`:

### 1. El Tronco y la Base Matemática (Fundamentos abstractos)
*Modifican y definen los axiomas del espacio vectorial y tensorial puro.*
- 1.1. Lógica Matemática
- 1.2. Álgebra Abstracta y Geometría Diferencial (Deformación de la matriz `[x,y,z,w]`)
- 1.3. Cálculo Avanzado y Ecuaciones Diferenciales
- 1.4. Teoría de la Probabilidad y Estadística
- 1.5. Teoría de la Información y Sistemas Dinámicos

### 2. Capa de la Física (Leyes fundamentales del universo)
*Definen cómo la masa y la energía curvan y se propagan en el espacio-tiempo geométrico.*
- 2.1. Física Cuántica y Mecánica de Matrices
- 2.2. Teoría Cuántica de Campos
- 2.3. Electrodinámica Cuántica
- 2.4. Cromodinámica Cuántica
- 2.5. Relatividad General y Especial
- 2.6. Mecánica Estadística
- 2.7. Termodinámica de Sistemas Fuera del Equilibrio
- 2.8. Física del Estado Sólido y Materia Condensada

### 3. Intermediario: Física-Química (El puente atómico y molecular)
*Construyen la primera noción de materia e interacción espectral (comienza el mapeo fotónico hacia `[r,g,b,a]`).*
- 3.1. Física Atómica
- 3.2. Física Molecular
- 3.3. Química Cuántica
- 3.4. Fisicoquímica
- 3.5. Espectroscopia Molecular
- 3.6. Mecánica Estadística Molecular
- 3.7. Cinética Química y Termodinámica Química
- 3.8. Electroquímica

### 4. Capa de la Química (Estructuras, enlaces y reacciones)
*Define cómo las interacciones fotónicas y materiales modifican definitivamente la óptica superficial y volumétrica.*
- 4.1. Química Inorgánica
- 4.2. Química Orgánica
- 4.3. Química de Coordinación
- 4.4. Química Supramolecular
- 4.5. Química de Polímeros
- 4.6. Cristalografía
- 4.7. Química Teórica y Computacional

### 5. Intermediario: Química-Biología (El puente de la materia viva)
*Establece los primeros bloques de repetición algorítmica y adaptación sobre la forma física.*
- 5.1. Bioquímica
- 5.2. Bioquímica Estructural
- 5.3. Biología Molecular
- 5.4. Biología Química
- 5.5. Biofísica Molecular
- 5.6. Enzimología
- 5.7. Genómica Química y Metabolómica

### 6. Capa de la Biología (Sistemas vivos y complejidad emergente)
*Gobierna patrones, L-Systems, fractales adaptativos y desarrollo morfogénico en función del tiempo `w`.*
- 6.1. Biología Celular y Citología
- 6.2. Genética y Epigenética
- 6.3. Biología del Desarrollo y Morfogénesis
- 6.4. Fisiología (Sistémica y Celular)
- 6.5. Neurobiología y Neurociencia Computacional
- 6.6. Biología Evolutiva
- 6.7. Ecología de Poblaciones y Comunidades
- 6.8. Biología de Sistemas

### 7. Capa Macro-Sistémica (Arquitectura del Mundo y Geociencias)
*Simulan comportamientos ambientales y estructurales a gran escala usando ruido fractal, erosión volumétrica y dinámicas de fluidos globales.*
- 7.1. Geología y Geomorfología (Orogénesis, erosión iterativa)
- 7.2. Meteorología y Climatología (Sistemas de fluidos, nubes volumétricas)
- 7.3. Oceanografía y Dinámica de Fluidos planetaria
- 7.4. Ecología de Paisajes y Biomas (Mapeo de ecosistemas realistas)

### 8. Capa Cosmológica (El Macro-Universo)
*Las leyes aplicadas a la métrica estelar, astrofísica y el espacio profundo, cerrando el bucle de la escala.*
- 8.1. Mecánica Celeste y Órbitas (Gravedad orbital temporal)
- 8.2. Astrofísica Estelar (Radiación de cuerpo negro, espectros estelares)
- 8.3. Cosmología Física y Nebulosas (Mecánica de gases interestelares)

---

## 3. Instrucciones Estrictas para Agentes IA

Si eres un agente convocado para crear una de estas librerías (`std_*.hm`), debes atenerte a las siguientes reglas inquebrantables:

1. **PROHIBIDO EL ESTADO MUTABLE:** Nunca asumas variables que guardan estado. Cualquier evolución debe depender única y exclusivamente de las coordenadas de entrada `(x, y, z, w)` del punto evaluado.
2. **PROHIBIDO HARDCODEAR EN C++:** No puedes sugerir modificaciones en el compilador C++ (`herm_eval`, `renderer.cpp`, etc.). Todo el dominio científico *debe* residir en el código fuente `.hm`.
3. **HERENCIA OBLIGATORIA DE CAPA INFERIOR:** No puedes implementar una librería de capa N sin haber leído y consumido las `def`s expuestas por las librerías de capa N-1.
4. **COMPATIBILIDAD TENSORIAL ESTRICTA:**
   - Las funciones de **Forma/Movimiento** solo modifican o retornan valores ligados a `[x, y, z, w]` (índices 0..3).
   - Las funciones de **Óptica/Color** solo modulan `[r, g, b, a]` (índices 4..7).
   - Un agente **nunca mezcla ambas**: no puede devolver color dentro de una función geométrica, ni modificar geometría dentro de una función de shading.

---

## 4. Workflow del Agente: Las 5 Fases

Cada agente asignado a una subciencia debe seguir estas fases **en orden estricto**. No se salta ninguna.

### Fase 1 — Investigación Científica
- Identifica la **ecuación canónica** del fenómeno (diferencial, integral o algebraica).
- Busca la fuente primaria: papers, libros de texto, formularios físicos.
- **Ejemplo:** Para la reflectancia de Fresnel, la ecuación es: `F(θ) = ((n1·cos(θi) - n2·cos(θt)) / (n1·cos(θi) + n2·cos(θt)))²`

### Fase 2 — Reducción a Forma Cerrada
- Elimina integrales no computables en tiempo real.
- Busca la **aproximación analítica cerrada** más usada en la literatura (Schlick, Beckmann, etc.).
- Documenta el **error de aproximación** y en qué condiciones es aceptable.
- **Ejemplo Fresnel (Schlick):** `F(v, n) = F0 + (1 - F0) * pow(1 - dot(v, n), 5)`

### Fase 3 — Traducción al Álgebra Hermética
- Convierte la fórmula cerrada usando **solo** los operadores disponibles del lenguaje:
  - Funciones matemáticas: `sin, cos, tan, asin, acos, atan, pow, sqrt, abs, exp, log`
  - Operadores de comparación: `max, min, clamp, step, smoothstep`
  - Álgebra lineal: `dot, length, normalize, cross` *(si el evaluador las expone)*
  - Variables de entorno: `x, y, z, w, pi, e`
  - Variables de contexto óptico: `nx, ny, nz` (normal), `lx, ly, lz` (luz), `vx, vy, vz` (cámara/vista)
- Escribe las fórmulas como `let` + `def` encadenados.

### Fase 4 — Redacción del Archivo `.hm`
- Escribe el archivo siguiendo la **Plantilla Canónica** de la Sección 5.
- Añade un bloque de comentarios en cabecera con: fuente científica, versión, autor (el agente), dependencias.
- Expone únicamente las `def`s de interfaz pública que otras capas consumirán.

### Fase 5 — Validación y Prueba
- Escribe un archivo `.herm` de prueba que use la librería.
- Verifica que el compilador emite el `.rih` sin errores de tipo.
- Verifica que el render produce un resultado visualmente coherente con el fenómeno simulado.
- Documenta las **métricas de overhead** (tiempo extra de render vs escena base).

---

## 5. Plantilla Canónica de Librería `.hm`

Todo archivo de librería debe tener esta estructura. Los agentes no pueden desviarse de ella.

```hermetico
// ============================================================
// LIBRERÍA: std_<capa>_<subciencia>.hm
// CAPA:     <Nombre de la capa del Árbol Ontológico>
// VERSIÓN:  0.1
// FUENTE:   <Referencia al paper o formulario canónico>
// DEPS:     <Lista de librerías de capa inferior que se importan>
// ============================================================

// --- Constantes Fundamentales ---
// Solo constantes físicas o matemáticas puras. Sin coordenadas.
let PI2 = pi * 2;
let INV_PI = 1 / pi;

// --- Funciones Internas (privadas, prefijo '_') ---
// Auxiliares que no forman parte de la interfaz pública.
def "_aux_funcion"(a, b) { ... }

// --- Interfaz Pública ---
// Las def que otras capas superiores PUEDEN consumir.
// Nombradas en snake_case descriptivo.
def "nombre_del_fenomeno"(param1, param2) {
    // Fórmula algebraica pura.
    // Cada línea debe ser legible y trazable a la Fase 1.
    ...
}
```

---

## 6. Contrato de Interfaz entre Capas

Este es el **protocolo de comunicación** que define qué variables puede consumir una capa superior y cuáles expone hacia arriba. Es el "sistema de tipos" del árbol ontológico.

| Capa Productora | Variable Expuesta | Tipo | Descripción |
|---|---|---|---|
| Evaluador C++ (base) | `x, y, z` | `f32` | Coordenadas del punto en el rayo actual |
| Evaluador C++ (base) | `w` | `f32` | Tiempo / cuarta dimensión |
| Evaluador C++ (base) | `nx, ny, nz` | `f32` | Normal de superficie en el punto |
| Evaluador C++ (base) | `lx, ly, lz` | `f32` | Dirección de la luz principal |
| Evaluador C++ (base) | `vx, vy, vz` | `f32` | Vector de vista (cámara → punto) |
| **Capa Matemática** | Funciones de álgebra pura | `def` | `lerp`, `remap`, `clamp_norm`, etc. |
| **Capa Física** | Normal de deformación | `def` | `deform_normal(nx, ny, nz, ...)` |
| **Capa Física** | SDFs de forma compuesta | `def` | `sdf_wave(amp, freq)`, etc. |
| **Intermediario Fís-Quím** | Factor de reflectancia | `def` | `fresnel(ior)`, `schlick(f0)` |
| **Capa Química** | Color espectral modulado | `def` | `spectral_metal(r, g, b, ior)` |
| **Intermediario Quím-Biol** | Patrón de repetición | `def` | `cell_pattern(freq, seed)` |
| **Capa Biológica** | Distribución morfogénica | `def` | `branch(iter, angle, w)` |
| **Capa Macro-Sistémica**| Campo de elevación / Clima | `def` | `terrain_height(x, y, z)`, `weather_density(...)` |
| **Capa Cosmológica**  | Coordenadas celestes | `def` | `orbit_pos(w, mass)`, `star_color(...)` |

> **Regla de oro:** Una capa solo puede llamar `def`s de su misma capa o de una capa inferior. **Nunca hacia arriba.** Esto garantiza que el grafo de dependencias es un DAG (sin ciclos).

---

## 7. Mapa de Archivos de Librería (Naming Convention)

Los agentes deben nombrar sus archivos de forma estrictamente predecible:

```
libreria/
├── std_math_algebra.hm          # Capa 1.2 — Álgebra vectorial y tensorial
├── std_math_calculus.hm         # Capa 1.3 — Derivadas, gradientes
├── std_math_noise.hm            # Capa 1.4/1.5 — Hash, Perlin, Simplex
│
├── std_phys_sdf.hm              # Capa 2 — SDFs primitivas y operadores
├── std_phys_kinematics.hm       # Capa 2 — Movimiento, trayectorias en W
├── std_phys_relativity.hm       # Capa 2.5 — Curvatura espacio-tiempo
├── std_phys_thermo.hm           # Capa 2.7 — Calor, difusión, entropía
│
├── std_bridge_atomic.hm         # Capa 3.1/3.2 — Física atómica y molecular
├── std_bridge_spectral.hm       # Capa 3.5 — Espectros de emisión/absorción
├── std_bridge_electrochem.hm    # Capa 3.8 — Electroquímica
│
├── std_chem_optics.hm           # Capa 4 — Fresnel, BRDF, refracción
├── std_chem_crystal.hm          # Capa 4.6 — Anisotropías cristalinas
├── std_chem_polymer.hm          # Capa 4.5 — Plásticos, geles, fibras
│
├── std_bridge_biochem.hm        # Capa 5.1/5.2 — Proteínas, membranas
├── std_bridge_molbio.hm         # Capa 5.3 — ADN, replicación morfogénica
│
├── std_bio_morpho.hm            # Capa 6.3 — Morfogénesis y L-Systems
├── std_bio_neuro.hm             # Capa 6.5 — Patrones neuronales
├── std_bio_evolution.hm         # Capa 6.6 — Variación y selección iterativa
├── std_bio_systems.hm           # Capa 6.8 — Sistemas biológicos entrelazados
│
├── std_world_geo.hm             # Capa 7.1 — Ruido de terreno, fractales, erosión
├── std_world_climate.hm         # Capa 7.2 — Sistemas de nubes volumétricas
├── std_world_biomes.hm          # Capa 7.4 — Distribución de hábitats y flora
│
├── std_cosmo_orbits.hm          # Capa 8.1 — Mecánica de sistemas estelares
└── std_cosmo_stars.hm           # Capa 8.2 — Espectros estelares y nebulosas
```

---

## 8. Tarea Tipo para un Agente (Ejemplo de Asignación)

A continuación se muestra cómo debe ser formulada la tarea para un agente específico:

```
ROL: Eres un experto en Óptica Física y renderizado PBR.
CONTEXTO: Lee docs/11-protocolo-librerias.md y docs/04-guia-uso.md.
TAREA: Implementa la librería `libreria/std_chem_optics.hm` para la Capa 4.
DEPENDENCIAS: Puedes usar los `def`s de `std_math_algebra.hm` y `std_bridge_spectral.hm`.
FENÓMENO OBJETIVO: Fresnel (Schlick), distribución GGX de microfacetas, y Beer-Lambert volumétrico.
FASES OBLIGATORIAS: Sigue las 5 fases del Protocolo (Sección 4). No te saltes la Fase 1.
ENTREGA: El archivo .hm + un archivo de prueba `ejemplos/test_optics.herm`.
PROHIBIDO: No toques ningún archivo C++. No uses propiedades PBR aisladas (metallic, roughness).
```

---

> *"Lo que está abajo es como lo que está arriba, y lo que está arriba es como lo que está abajo,*
> *para obrar los milagros de una sola cosa (las Matemáticas)."*
