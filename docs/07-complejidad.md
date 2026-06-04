# Complejidad Computacional

## 1. Compilación (.herm → RIH)

### 1.1 Parse

$$O(n \cdot k)$$

- $n$ = número de tokens
- $k$ = profundidad de anidamiento del AST
- En la práctica: $O(n)$ lineal, escaneo único con pila

### 1.2 Resolución semántica

$$O(m \cdot h + i \cdot d)$$

- $m$ = número de materiales
- $h$ = profundidad de la jerarquía para herencia
- $i$ = número de instancias
- $d$ = profundidad de la jerarquía `def`

### 1.3 Evaluación de expresiones

$$O(e \cdot f \cdot w)$$

- $e$ = número de expresiones en el escenario
- $f$ = número de fotogramas (W)
- $w$ = tamaño del árbol de expresión (peor caso: profundidad del AST de expresión)

## 2. Renderizado (Ray Marching)

### 2.1 Sphere Tracing por píxel

$$O(s \cdot n \cdot p)$$

- $s$ = pasos de sphere tracing (típico: 20-128)
- $n$ = número de nodos SDF en el grafo de escena
- $p$ = número de píxeles = $X \cdot Y$

**Peor caso:** $O(128 \cdot N_{nodos} \cdot X \cdot Y)$

### 2.2 Evaluación del árbol SDF

$$O(t)$$

- $t$ = número de nodos en el árbol SDF de la escena
- En cada paso de sphere tracing se evalúa el árbol completo (en escena plana)
- Potencialmente optimizable con BVH para $O(\log n)$

### 2.3 Cálculo de normal

$$O(6 \cdot s \cdot n)$$

- 6 evaluaciones de SDF para gradiente numérico centrado
- $(f(x+\varepsilon) - f(x-\varepsilon)) / (2\varepsilon)$ para cada eje

### 2.4 Sombra

$$O(s_s \cdot n)$$

- $s_s$ = pasos de shadow ray (típico: 32)
- $n$ = número de nodos

## 3. Complejidad Espacial

### 3.1 Grafo de escena (RIH en memoria)

$$O(m + d + n)$$

- $m$ = materiales
- $d$ = definiciones
- $n$ = nodos

### 3.2 Buffer de render

$$O(X \cdot Y \cdot 4)$$

- 4 bytes por píxel (RGBA)
- 800×600 = 1.92 MB
- 1920×1080 = 8.29 MB

### 3.3 Grafo de escena expandido (flatten)

$$O(n_{instancias} + n_{grupos} + n_{hojas})$$

- Cada instancia se expande a un nodo hoja independiente
- Cada grupo se expande fusionando sus transformaciones
- La memoria es lineal con el número de nodos efectivos

## 4. Formato .tb (Disco)

### 4.1 Compresión por bloques 8×8

Caso ideal (color sólido): 4 bytes por bloque = $X/8 \cdot Y/8 \cdot 4$

Caso peor (RAW): 256 bytes por bloque = $X/8 \cdot Y/8 \cdot 256$

Típico: mezcla de P0 + PAL_RGBA + RAW:

$$|.tb| \approx B_{P0} \cdot 4 + B_{PAL} \cdot (4 + C_{paleta}) + B_{RAW} \cdot 256$$

- $B_{P0}$ = bloques de color sólido
- $B_{PAL}$ = bloques con paleta
- $B_{RAW}$ = bloques sin comprimir

## 5. Animación (Múltiples Fotogramas)

### 5.1 Sin caché (re-evaluación por fotograma)

$$O(f \cdot (C_{parse} + C_{eval} + C_{render}))$$

- $f$ = número de fotogramas
- Cada fotograma re-evalúa todas las expresiones
- Render independiente por fotograma

### 5.2 Con pre-evaluación (ideal)

$$O(C_{parse} + f \cdot C_{eval} + f \cdot C_{render})$$

- Parse único
- Evaluación de expresiones por fotograma
- Render independiente por fotograma

## 6. Comparativas

### 6.1 vs Renderizado poligonal tradicional

| Aspecto | SDF (Hermético) | Polígonos (GPU clásica) |
|---------|-----------------|------------------------|
| Memoria geometría | $O(n)$ (árbol SDF) | $O(v + t)$ (vértices + triángulos) |
| Resolución | Independiente | Dependiente de teselación |
| Anti-aliasing | Natural (campo continuo) | Necesita MSAA |
| Complejidad por píxel | $O(s \cdot n)$ | $O(1)$ (rasterización) |
| Curvatura infinita | Sí | No (límite de teselación) |

### 6.2 vs Vóxeles

| Aspecto | SDF | Vóxeles |
|---------|-----|---------|
| Memoria | $O(1)$ por primitiva | $O(X \cdot Y \cdot Z)$ |
| Resolución | Infinita | Limitada por grid |
| 4D | Natural | $O(X \cdot Y \cdot Z \cdot W)$ — maldición dimensional |

## 7. Cuellos de Botella Identificados

1. **Render multi-fotograma**: $O(f \cdot X \cdot Y \cdot s \cdot n)$ limita animaciones largas
2. **Evaluación SDF escena plana**: $O(s \cdot n)$ por píxel sin BVH
3. **Expresiones re-evaluadas**: sin precomputación, cada fotograma repite trabajo
4. **Formato .tb 8×8**: compresión aceptable pero overhead de bloques RAW en zonas detalladas

## 8. Optimizaciones Pendientes

| Técnica | Complejidad nueva | Ganancia esperada |
|---------|------------------|-------------------|
| BVH por nodos SDF | $O(s \cdot \log n)$ | 5-10× en escenas densas |
| Rejilla espacial | $O(s \cdot 1)$ | 10-50× |
| Pre-evaluación de expresiones | $O(f + C_{eval})$ | 2-5× en animaciones |
| Caché de fotogramas | $O(1)$ por frame re-render | Variable |
| AABB culling | $O(s \cdot n_{visible})$ | 2-5× |
