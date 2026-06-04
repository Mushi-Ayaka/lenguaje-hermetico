# Fundamentos Matemáticos

## 1. El Espacio de Estado

El universo se modela como un campo vectorial continuo en 4 dimensiones. Cada punto $\mathbf{p} = [X, Y, Z, W] \in \mathbb{R}^4$ tiene un estado interno:

$$\mathcal{C}: \mathbb{R}^4 \rightarrow \mathcal{S}$$

### 1.1 Estado interno vs salida visible

| Capa | Representación | Propósito |
|------|---------------|-----------|
| Estado interno | $(d, m, \mathbf{c}, A, P)$ | Distancia, material, color base, opacidad, propiedades |
| Salida colapsada | $[R, G, B, A]$ | Lo que ve el renderizador |

El colapso se define como:

$$\text{collapse}(d, m, \mathbf{c}, A, P) = [R, G, B, A]$$

### 1.2 El Vacío

En ausencia de entidades, todo punto tiene estado de vacío:

$$\mathbf{s}_{vacio} = (d = +\infty, m = \emptyset, \mathbf{c} = [0,0,0], A = 0, P = \emptyset)$$

## 2. Campos de Distancia Firmados (SDF)

Las entidades se definen mediante funciones $\Phi: \mathbb{R}^3 \rightarrow \mathbb{R}$ que calculan la distancia al límite más cercano.

| Signo | Significado |
|-------|-------------|
| $\Phi(\mathbf{p}) < 0$ | Dentro del objeto |
| $\Phi(\mathbf{p}) = 0$ | En la superficie |
| $\Phi(\mathbf{p}) > 0$ | Fuera del objeto |

### 2.1 Sólido vs Volumen

- **Sólido**: $A(\mathbf{p}) = H(-\Phi(\mathbf{p}))$ (Heaviside: 1 dentro, 0 fuera)
- **Volumen**: $A(\mathbf{p}) = \sigma(\Phi(\mathbf{p}))$ (densidad continua)

## 3. Los Tres Operadores Fundamentales

### 3.1 Perturbación ($\mathcal{P}$)

Inyecta presencia en el vacío: crea una entidad con forma, material y propiedades.

$$\mathcal{P}(\mathcal{C}): \mathbf{s} = f(\mathbf{p})$$

### 3.2 Influencia ($\mathcal{I}$)

Modifica una entidad existente: cambia color, material, densidad.

$$\mathcal{I}(\mathcal{C}): \mathbf{s}_{nueva} = \text{transform}(\mathbf{T}, \mathbf{s}_{previa})$$

### 3.3 Aniquilación ($\mathcal{K}$)

Elimina presencia sin alterar coordenadas: $A \rightarrow 0$, $d \rightarrow +\infty$.

$$\mathcal{K}(\mathcal{C}): \mathbf{s}_{nueva} = (+\infty, \emptyset, [0,0,0], 0, \emptyset)$$

## 4. Pirámide Física-Química-Biología

| Capa | Campos | Responsabilidad |
|------|--------|-----------------|
| **Física** | $(d, A, T_{local}, \text{bbox}, \text{motion})$ | Forma, ocupación, transformaciones, movimiento |
| **Química** | $(m, \mathbf{c}, P, \text{mode})$ | Material, color, propiedades ópticas, modo sólido/volumen |
| **Biología** | $(\text{parent}, \text{children}, T_{hierarchy}, \text{rules})$ | Jerarquías, herencia, instancias, reglas de ensamblaje |

### 4.1 Composición

$$\mathcal{R}(\mathbf{p}) = \text{compose}(\mathcal{F}(\mathbf{p}), \mathcal{Q}(\mathbf{p}), \mathcal{B}(\mathbf{p}))$$

### 4.2 Selección de superficie (resolve)

Para la unión de dos campos $s_1 = (d_1, m_1, \mathbf{c}_1, A_1)$, $s_2 = (d_2, m_2, \mathbf{c}_2, A_2)$:

$$d_r = \min(d_1, d_2)$$

$$m_r = \begin{cases} m_1 & d_1 < d_2 \\ m_2 & d_2 < d_1 \end{cases}$$

$$\mathbf{c}_r = \begin{cases} \mathbf{c}_1 & d_1 < d_2 \\ \mathbf{c}_2 & d_2 < d_1 \end{cases}$$

$$A_r = \begin{cases} A_1 & d_1 < d_2 \\ A_2 & d_2 < d_1 \end{cases}$$

### 4.3 Reglas de empate

- Si $\|d_1 - d_2\| \leq \varepsilon$ y uno es `solid`: gana `solid`.
- Si ambos `volume`: composición volumétrica (Beer-Lambert).
- Si empate exacto: prioridad determinista por `material_id`.

## 5. Tiempo como Geometría (Eje W)

El tiempo es la cuarta dimensión espacial. No hay simulación: el universo es un bloque 4D.

**Movimiento**: deformación del campo de distancia a lo largo de $W$:

$$\mathbf{c}(W) = \mathbf{x}_0 + \mathbf{v} \cdot W$$

**Renderizado**: corte transversal en $W = g(t)$:

$$F_{gte}(x, y, t) = \text{collapse}\left(\mathcal{C}\left(\mathbf{C}^{-1}(x, y), Z_{depth}, g(t)\right)\right)$$

## 6. Proyección Observacional

La cámara mapea $(X, Y, Z) \rightarrow (x_{screen}, y_{screen})$ mediante el operador $\mathcal{O}$:

- **Matriz de cámara $\mathbf{C}$**: proyección perspectiva desde `position` hacia `target`.
- **Función cronológica $g$**: mapea $t_{frame} \rightarrow W$.

## 7. Herencia de Transformaciones

En una jerarquía de nodos, la transformación efectiva se compone por acumulación:

$$T_{efectiva} = T_{raíz} \cdot T_{padre} \cdot T_{local}$$

- Traslación y rotación: **aditivas** (se suman a la del padre).
- Escala: **multiplicativa** (se multiplica por la del padre).

## 8. Expresiones Analíticas

Toda función en el lenguaje debe ser analítica y determinista. No se admite:

- Ruido estadístico (no determinsita)
- Redes neuronales (aproximación no certificable)
- Funciones con estado oculto

Se permite:

- Funciones trigonométricas: $\sin, \cos, \tan$
- Álgebra: $+, -, *, /, ^$
- Límites: $\min, \max, \text{clamp}, \text{abs}$
- Interpolación: $\text{lerp}, \text{mix}, \text{smoothstep}$
- Vectoriales: $\text{length}, \text{dot}, \text{cross}, \text{normalize}$

## 9. Materiales (Química PBR)

Cada material define:

| Propiedad | Rango | Descripción |
|-----------|-------|-------------|
| `base_color` | $[0,1]^3$ | Color base RGB |
| `roughness` | $[0,1]$ | Rugosidad superficial |
| `metallic` | $[0,1]$ | Factor metálico |
| `emission` | $[0,\infty)^3$ | Emisión luminosa |
| `ior` | $[1, \infty)$ | Índice de refracción |
| `transmission` | $[0,1]$ | Transmisión (transparencia) |
| `opacity` | $[0,1]$ | Opacidad |
| `subsurface` | $[0,1]$ | Dispersión subsuperficial |

## 10. Renderizado: Sphere Tracing (Ray Marching)

El renderizador convierte el campo SDF en una imagen usando **sphere tracing**, una variante de ray marching que aprovecha la propiedad de distancia firmada de las SDFs.

### 10.1 Algoritmo base (SDF simple)

Dado un rayo $\mathbf{r}(t) = \mathbf{o} + t \cdot \mathbf{d}$, para cada píxel:

1. Inicializar $t = t_{min}$ (cerca del plano focal).
2. En cada paso, evaluar $d = \Phi(\mathbf{r}(t))$ en el punto actual del rayo.
3. Si $|d| < \varepsilon$: **superficie encontrada**. Calcular normal, sombra, color.
4. Si $d > 0$: avanzar $t = t + d$ (sphere tracing — la SDF garantiza que no hay superficie más cerca).
5. Si $d < 0$: estamos dentro de un objeto — retroceder o marcar como error (no debería ocurrir con normales bien orientadas).
6. Si $t > t_{max}$: **sin impacto**. Devolver color de fondo.

```
t = t_min
while t < t_max:
    p = o + t * d
    d_actual = sdf(p)
    if |d_actual| < epsilon:  → HIT
        return shade(p, normal(p), material)
    t += d_actual
return bg_color
```

Cada paso avanza **exactamente la distancia a la superficie más cercana** — de ahí el nombre "sphere tracing": se trazan esferas de radio $d$ centradas en el punto actual, y la siguiente evaluación está en la superficie de esa esfera. Esto garantiza que nunca se salta una superficie.

### 10.2 Árbol SDF (SDF compleja)

En escenas con múltiples nodos, la SDF no es una sola primitiva sino un **árbol de operaciones**:

```
Φ_escena(p) = min(Φ_esfera(p - T_1), min(Φ_caja(p - T_2), Φ_cilindro(p - T_3)))
```

El sphere tracing evalúa **todo el árbol** en cada paso:

```
d_actual = INF
for each nodo in escena:
    p_local = aplicar_transform(p, nodo.transform)
    d = evaluar_arbol_SDF(nodo.sdf_tree, p_local)
    d *= min(nodo.escala)     // ajuste por escala no uniforme
    d_actual = min(d_actual, d)

if |d_actual| < epsilon → HIT, material = material del nodo ganador
```

### 10.3 Complejidad

| Escenario | Pasos por píxel | Evaluaciones por paso |
|-----------|----------------|----------------------|
| 1 esfera aislada | ~30-60 | 1 SDF |
| Escena simple (5 nodos) | ~30-60 | 5 SDFs |
| Escena densa (40 nodos) | ~30-60 | 40 SDFs |
| Con BVH (ideal) | ~30-60 | $O(\log n)$ SDFs |

El número de pasos ($s$) es independiente de $n$ porque la SDF combinada sigue siendo una función de distancia válida. El costo por paso escala $O(n)$ en escena plana, $O(\log n)$ con BVH.

### 10.4 Normal, sombra, luces

**Normal**: gradiente numérico por diferencias finitas:

$$\nabla\Phi(\mathbf{p}) \approx \frac{1}{2\varepsilon}\begin{pmatrix}
\Phi(\mathbf{p}+\varepsilon_x) - \Phi(\mathbf{p}-\varepsilon_x)\\
\Phi(\mathbf{p}+\varepsilon_y) - \Phi(\mathbf{p}-\varepsilon_y)\\
\Phi(\mathbf{p}+\varepsilon_z) - \Phi(\mathbf{p}-\varepsilon_z)
\end{pmatrix}$$

Son 6 evaluaciones extra de SDF. Se reutiliza el mismo árbol SDF completo.

**Sombra** (shadow ray): desde el punto de impacto, se lanza un segundo rayo hacia cada luz direccional. Si encuentra una superficie antes de llegar, el punto está en sombra (factor 0.3).

**Múltiples luces**: para cada luz en la escena, se calcula contribución Blinn-Phong independiente. Luces puntuales añaden atenuación $1/(1 + f \cdot d^2)$ pero no proyectan sombra (por ahora).

### 10.5 Volumen (modo volume)

Para nodos en modo `volume`, no se busca superficie sino que se integra densidad a lo largo del rayo:

```
T = 1.0   // transmitancia acumulada
color = [0, 0, 0]
for each paso:
    p = o + t * d
    densidad = densidad_del_volumen(p)
    if densidad > 0:
        absorcion = exp(-densidad * paso)
        color += emision(p) * T * (1 - absorcion)
        T *= absorcion
    if T < 0.01: break
    t += paso
```

Al final, el color es `color + T * color_solido_o_fondo` — composición Beer-Lambert.

## 11. Referencias

- `Representacion hermetica.md` — Versión extendida de estos fundamentos
- `03-especificaciones.md` — Especificación formal del lenguaje y RIH
- `contrato/herm.h` — Tipos C++ del compilador
