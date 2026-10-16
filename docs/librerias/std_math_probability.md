# std_math_probability.hm

**Capa:** 1.4 — Teoría de la Probabilidad y Estadística
**Versión:** 0.2
**Fuente:** Estadística matemática, distribuciones de probabilidad
**Dependencias:** std_math_core.hm
**Fecha:** 5 de junio de 2026

## Cómo

`std_math_probability.hm` implementa funciones de densidad de probabilidad (PDFs) y funciones de distribución acumulativa (CDFs) para distribuciones clásicas, funciones especiales de probabilidad y estadística, estadísticas descriptivas, procesado de señales probabilístico (entropía, información mutua, divergencia KL) y un kernel de densidad 2D. También define una SDF de iso-superficie gaussiana 3D.

## Por qué

El arte generativo a menudo requiere elementos aleatorios o ruidosos, pero Hermético es puramente determinista. Esta librería provee las herramientas para modelar incertidumbre, variabilidad y distribuciones estadísticas de manera algebraica, permitiendo desde texturas estocásticas hasta simulaciones de fenómenos naturales con apariencia aleatoria pero completamente deterministas.

## Para qué

Se usa para generar números pseudo-aleatorios con distribución específica (normal, Cauchy, Pareto, Weibull, exponencial), estimar densidad de puntos en 2D (kde2d), medir entropía e información mutua en señales generadas, calcular estadísticas descriptivas (varianza, desviación estándar, coeficiente de variación, asimetría), aproximar funciones especiales (erf, gamma, beta) y construir superficies de iso-probabilidad como elipsoides de confianza gaussianos.

## Funciones principales

| Nombre | Descripción breve |
|---|---|
| `normal_pdf(x, μ, σ)` | PDF de la distribución normal |
| `std_normal_pdf(x)` | PDF de la normal estándar N(0,1) |
| `normal_cdf_approx(x)` | CDF aproximada de la normal estándar |
| `cauchy_pdf(x, x₀, γ)` | PDF de la distribución de Cauchy |
| `laplace_pdf(x, μ, b)` | PDF de la distribución de Laplace |
| `pareto_pdf(x, xₘᵢₙ, a)` | PDF de la distribución de Pareto (power law) |
| `exp_pdf(x, λ)` | PDF de la distribución exponencial |
| `weibull_pdf(x, k, λ)` | PDF de la distribución de Weibull |
| `erf_approx(x)` | Función error: erf(x) ≈ tanh(1.2533·x) |
| `gamma_stirling(n)` | Función Gamma por aproximación de Stirling |
| `beta_approx(a, b)` | Función Beta: B(a,b) = Γ(a)Γ(b)/Γ(a+b) |
| `variance(x₁, x₂, x₃, μ)` | Varianza de 3 muestras |
| `std_dev(x₁, x₂, x₃, μ)` | Desviación estándar de 3 muestras |
| `coef_var(σ, μ)` | Coeficiente de variación: σ/|μ| |
| `skewness3(x₁, x₂, x₃, μ, σ)` | Asimetría (skewness) de 3 muestras |
| `entropy3(p₁, p₂, p₃)` | Entropía de Shannon para distribución de 3 estados |
| `mutual_info(p_xy, p_x, p_y)` | Información mutua: p_xy · log(p_xy / (p_x·p_y)) |
| `kl_divergence(p_x, q_x)` | Divergencia KL: p · log(p/q) |
| `kde2d(sx, sy, qx, qy, bw)` | Kernel de densidad gaussiano 2D |

## SDFs definidas

| Nombre | Descripción |
|---|---|
| `gaussian_isosurface(μx, μy, μz, σ, iso)` | Iso-superficie gaussiana 3D (elipsoide de confianza) |

## Notas

- `normal_cdf_approx` usa la aproximación de Abramowitz & Stegun basada en tanh.
- `entropy3` usa log₂ (base 2) para entropía en bits. Internamente divide log() por log(2).
- Las funciones estadísticas (variance, std_dev, skewness3) están limitadas a 3 muestras (sin loops en Hermético).
- `pareto_pdf` y `exp_pdf` usan `select` para garantizar que la PDF sea 0 fuera del soporte.
- `gamma_stirling` aproxima Γ(n) solo para n > 0; no es precisa para n pequeños.
- `beta_approx` compone tres llamadas a `gamma_stirling`; hereda sus errores de aproximación.
