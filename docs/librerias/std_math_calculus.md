# std_math_calculus.hm

**Capa:** 1.3 — Cálculo Avanzado y Ecuaciones Diferenciales
**Versión:** 0.2
**Fuente:** Análisis matemático, ODEs, series de Taylor, integrales
**Dependencias:** std_math_core.hm, std_math_algebra.hm
**Fecha:** 5 de junio de 2026

## Cómo

`std_math_calculus.hm` implementa cálculo analítico y numérico sobre el dominio de SDFs. Incluye derivadas analíticas de funciones elementales (sin, cos, exp, ln, pow, sqrt, tan), gradiente numérico por diferencias finitas, expansiones de Taylor (sin orden 7, cos orden 6, exp orden 4, ln1+ orden 4, sqrt1+ orden 3), integrales numéricas (trapecio, Simpson 1/3) y analíticas (pow, sin, cos, exp), soluciones cerradas de ecuaciones diferenciales ordinarias (crecimiento/decaimiento exponencial, oscilador armónico simple y amortiguado, crecimiento logístico, onda 1D viajera y estacionaria, onda amortiguada, difusión de calor 1D), análisis de Fourier (armónicos de onda cuadrada), cálculo de variaciones (energía cinética, potencial gravitatorio, lagrangiana), interpolación hermítica C¹ (posición y velocidad), transformadas de Laplace analíticas (exp, seno, coseno) y SDFs de curvas calculus (cicloide, espiral de Arquímedes).

## Por qué

Cubre la brecha entre geometría estática (capas 1.1–1.2) y la simulación de movimiento (capas 1.4–1.5). Sin cálculo no hay cinemática, sin ODEs no hay sistemas dinámicos, sin series de Taylor no hay aproximaciones eficientes. Es el puente entre forma y evolución temporal.

## Para qué

Se usa para animar formas con movimiento físicamente inspirado (oscilaciones, ondas, crecimiento logístico), aproximar funciones costosas con polinomios de Taylor de bajo orden, integrar campos escalares sobre volúmenes SDF, interpolar trayectorias suaves con hermitas C¹, modelar superficies paramétricas (cicloide, espiral de Arquímedes) y calcular energías para sistemas lagrangianos.

## Funciones principales

| Nombre | Descripción breve |
|---|---|
| `d_sin(x)` | Derivada de sin: cos(x) |
| `d_cos(x)` | Derivada de cos: −sin(x) |
| `d_exp(x)` | Derivada de exp: exp(x) |
| `d_ln(x)` | Derivada de ln: 1/x |
| `d_pow(x, n)` | Derivada de xⁿ: n·xⁿ⁻¹ |
| `d_sqrt(x)` | Derivada de √x: 1/(2√x) |
| `d_tan(x)` | Derivada de tan: sec²(x) |
| `grad_num_{x,y,z}(h)` | Factor de escala para gradiente numérico |
| `sin_taylor7(x)` | sin(x) ≈ x − x³/6 + x⁵/120 − x⁷/5040 |
| `cos_taylor6(x)` | cos(x) ≈ 1 − x²/2 + x⁴/24 − x⁶/720 |
| `exp_taylor4(x)` | exp(x) ≈ 1 + x + x²/2 + x³/6 + x⁴/24 |
| `ln1p_taylor4(x)` | ln(1+x) ≈ x − x²/2 + x³/3 − x⁴/4 (|x|<1) |
| `sqrt1p_approx(x)` | √(1+x) ≈ 1 + x/2 − x²/8 + x³/16 |
| `trapezoid_rule(fa, fb, h)` | Regla del trapecio |
| `simpson_rule(fa, fm, fb, h)` | Regla de Simpson 1/3 |
| `int_pow(x, n)` | Integral de xⁿ: xⁿ⁺¹/(n+1) |
| `int_sin(x)` | Integral de sin: −cos(x) |
| `int_cos(x)` | Integral de cos: sin(x) |
| `int_exp(x)` | Integral de exp: exp(x) |
| `ode_exp_growth(y0, k, t)` | Crecimiento exponencial: y₀·eᵏᵗ |
| `ode_exp_decay(y0, k, t)` | Decaimiento exponencial: y₀·e⁻ᵏᵗ |
| `shm_pos(A, B, ω, t)` | Oscilador armónico: posición |
| `shm_vel(A, B, ω, t)` | Oscilador armónico: velocidad |
| `damped_shm(A, B, ω, ζ, t)` | Oscilador amortiguado (subamortiguado) |
| `logistic_growth(P₀, K, r, t)` | Crecimiento logístico: K/(1+((K−P₀)/P₀)·e⁻ʳᵗ) |
| `wave1d(A, k, ω, x, t)` | Onda viajera 1D: A·sin(kx − ωt) |
| `standing_wave(A, k, ω, x, t)` | Onda estacionaria: A·sin(kx)·cos(ωt) |
| `damped_wave(A, γ, ω, t)` | Onda amortiguada: A·e⁻ᵞᵗ·sin(ωt) |
| `heat1d_mode1(T₀, D, L, x, t)` | Difusión de calor 1D (modo fundamental) |
| `fourier_square_n(n, x, period)` | n-ésimo armónico de Fourier (onda cuadrada) |
| `fourier_square_approx4(x, period)` | Suma 4 armónicos impares (onda cuadrada) |
| `kinetic_energy(mass, vx, vy, vz)` | Energía cinética: ½mv² |
| `potential_gravity(G, M, m, r)` | Potencial gravitatorio: −GMm/r |
| `lagrangian(T, V)` | Lagrangiana: L = T − V |
| `hermite_pos(p0, p1, m0, m1, t)` | Interpolación hermítica C¹ (posición) |
| `hermite_vel(p0, p1, m0, m1, t)` | Interpolación hermítica C¹ (velocidad) |
| `laplace_exp(s, a)` | Transformada de Laplace de e⁻ᵃᵗ: 1/(s+a) |
| `laplace_sin(s, ω)` | Transformada de Laplace de sin(ωt): ω/(s²+ω²) |
| `laplace_cos(s, ω)` | Transformada de Laplace de cos(ωt): s/(s²+ω²) |

## SDFs definidas

| Nombre | Descripción |
|---|---|
| `cycloid_curve(r, t_param)` | Curva cicloide generada por círculo de radio r |
| `archimedes_spiral(a, thick)` | Espiral de Arquímedes: r = a·θ, con grosor |

## Notas

- Las derivadas analíticas son exactas (forma cerrada); el gradiente numérico requiere el paso de diferencias finitas h (por defecto 0.001).
- Las expansiones de Taylor son de orden fijo (hardcoded, sin loops).
- Las ODEs ofrecen solución cerrada; no hay integradores numéricos tipo Runge-Kutta — eso pertenece a la capa de dinámica (1.5).
- La transformada de Laplace se evalúa en el dominio frecuencial s, no en tiempo.
