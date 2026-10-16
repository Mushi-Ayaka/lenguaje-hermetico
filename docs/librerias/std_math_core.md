# std_math_core.hm

**Capa:** 1.1 — Base Matemática (Fundamentos abstractos)
**Versión:** 0.2
**Fuente:** Análisis matemático clásico, formas algebraicas cerradas
**Dependencias:** Ninguna (capa base del árbol ontológico)
**Fecha:** 5 de junio de 2026

## Cómo

`std_math_core.hm` es la librería fundamental del árbol ontológico. Define constantes universales (π, e, φ, etc.), funciones de comparación algebraica (heaviside, smoothstep), aritmética extendida (sq, cb, inv, sat), interpolación (lerp, remap, smootherstep), trigonometría extendida (sin_deg, tan_safe, sinc, hiperbólicas), generación de ondas (cuadrada, triangular, sierra, pulsos), estadísticas básicas (medias), funciones especiales de señal (gaussianas, sigmoides, laplace), geometría pura 2D/3D (distancia, producto punto, ángulos), combinación suave de SDFs (smin/smax), conversión de coordenadas (esféricas, polares, cartesianas), operaciones de color (luminancia, blend modes) y funciones de hash determinista base para ruido procedural. Todas las funciones son algebraicas puras — sin loops, sin condicionales imperativos, sin `return`.

## Por qué

Provee los fundamentos matemáticos sin los cuales ninguna otra capa puede operar. Toda expresión en Hermético reduce a álgebra; esta librería es el alfabeto de ese álgebra. Desde constantes universales hasta operadores de señal, establece el vocabulario mínimo para modelar fenómenos físicos, geométricos y estéticos en el espacio de SDFs.

## Para qué

Se usa como import base en prácticamente toda escena `.herm`. Cualquier cálculo que involucre trigonometría, interpolación suave, ondas, distancias, combinación de formas, conversión de coordenadas o blending de color depende de esta librería. Es el cimiento sobre el que se construyen las capas 1.2 a 1.6.

## Funciones principales

| Nombre | Descripción breve |
|---|---|
| `heaviside(x)` | Función escalón unitario (step(0, x)) |
| `heaviside_smooth(x, k)` | Escrón suave con ancho k |
| `sign_approx(x)` | Signo aproximado: -1, 0, +1 |
| `select(cond, a, b)` | Selección algebraica condicional |
| `sq(x)` | Cuadrado: x² |
| `cb(x)` | Cubo: x³ |
| `inv(x)` | Inverso seguro con EPSILON |
| `inv_safe(x, eps)` | Inverso con epsilon configurable |
| `rcp(x)` | Recíproco exacto: 1/x |
| `frac(x)` | Parte fraccionaria |
| `mod_unit(x)` | Módulo unitario [0,1) |
| `sat(x)` / `sat01(x)` | Saturar en [0,1] |
| `clamp_sym(x, r)` | Saturación simétrica [-r, r] |
| `round_to(x, n)` | Redondeo al múltiplo de n |
| `snap(x, n)` | Truncado al múltiplo inferior de n |
| `thresh(x, t)` | 1 si x ≥ t, 0 si no |
| `smootherstep(e0, e1, x)` | Interpolación C² (polinomio de 5º grado) |
| `lerp_sat(a, b, t)` | Lerp con saturación en t |
| `bilerp(a, b, c, d, u, v)` | Interpolación bilineal |
| `remap(x, a, b, c, d)` | Re-mapeo de rango [a,b]→[c,d] |
| `remap01(x, a, b)` | Normalización de [a,b] a [0,1] |
| `sin_deg(d)` / `cos_deg(d)` | Seno/Coseno en grados |
| `tan_safe(x)` | Tangente segura (con EPSILON) |
| `cot(x)` | Cotangente |
| `sec(x)` | Secante |
| `csc(x)` | Cosecante |
| `sinc(x)` | Seno cardinal sinc(πx) |
| `sinh_approx(x)` | Seno hiperbólico |
| `cosh_approx(x)` | Coseno hiperbólico |
| `tanh_approx(x)` | Tangente hiperbólica |
| `square_wave(x, period)` | Onda cuadrada |
| `triangle_wave(x, period)` | Onda triangular |
| `sawtooth_wave(x, period)` | Onda diente de sierra |
| `pulse_smooth(x, center, width, k)` | Pulso rectangular suavizado |
| `damped_osc(x, freq, decay)` | Oscilación amortiguada |
| `mean(a, b)` | Media aritmética |
| `geometric_mean(a, b)` | Media geométrica |
| `harmonic_mean(a, b)` | Media armónica |
| `power_mean(a, b, p)` | Media generalizada (potencia p) |
| `weighted_mean(a, b, w)` | Media ponderada |
| `gaussian(x, sigma)` | Campana gaussiana centrada en 0 |
| `gaussian_norm(x, mu, sigma)` | Gaussiana con media μ |
| `sigmoid(x)` | Función logística estándar |
| `sigmoid_k(x, k)` | Sigmoide con pendiente k |
| `laplace(x, b)` | Distribución de Laplace |
| `sinc_norm(x)` | Sinc normalizada |
| `len2(a, b)` / `len3(a, b, c)` | Longitud 2D/3D |
| `dist2d(...)` / `dist3d(...)` | Distancia entre puntos 2D/3D |
| `dot2(...)` / `dot3(...)` | Producto punto 2D/3D |
| `angle_between(...)` | Ángulo entre vectores 3D |
| `smin(a, b, k)` | Unión suave de SDF (opmin) |
| `smax(a, b, k)` | Intersección suave de SDF |
| `sph2cart_{x,y,z}(r, θ, φ)` | Esféricas a cartesianas |
| `pol2{x,z}(r, θ)` | Polares a cartesianas 2D |
| `cart2{r,theta,phi}(...)` | Cartesianas a esféricas |
| `luminance(r, g, b)` | Luminancia CIE |
| `blend_{add,mul,screen,overlay}(a, b)` | Modos de mezcla |
| `hash11(x)` | Hash determinista 1D→1D |
| `hash12(x, y)` | Hash determinista 2D→1D |
| `hash21_{x,y}(x, y)` | Hash determinista 2D→2D |
| `hash13(x, y, z)` | Hash determinista 3D→1D |

## SDFs definadas

Ninguna. Las SDFs de combinación (smin/smax) son funciones, no definiciones de formas.

## Notas

- Es la única librería sin dependencias. Todas las demás capas matemáticas importan `std_math_core.hm`.
- Las funciones de hash (hash11, hash12, etc.) son la base sobre la que se construye el ruido procedural en `std_math_noise.hm`.
- Las constantes EPSILON (1e-5) e INF (1e20) se usan como guardas de división y límites en toda la biblioteca.
