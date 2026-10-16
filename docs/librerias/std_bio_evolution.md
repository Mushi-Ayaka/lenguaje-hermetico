# std_bio_evolution.hm

**Capa:** 5.1 — Teoría de la Evolución y Selección Natural
**Versión:** 0.2
**Fuente:** Darwin, Fisher, Wright, Kimura — Modern Synthesis
**Dependencias:** std_math_core.hm, std_math_probability.hm
**Fecha:** 6 de junio de 2026

## Cómo
Modela genética de poblaciones con selección direccional (frecuencia alélica en tiempo t), carga genética, deriva génica (varianza de Wright), tamaño efectivo poblacional, índice de fijación FST, aptitud darwiniana relativa, coeficiente de selección, tasa reproductiva neta R0, crecimiento intrínseco (Euler-Lotka), dinámica ecológica logística y Lotka-Volterra, y epidemiología SIR. Las SDFs visualizan paisajes de aptitud y nichos ecológicos.

## Por qué
La evolución es el marco unificador de toda la biología. La selección natural, deriva y flujo génico moldean la diversidad y adaptación de las poblaciones a lo largo del tiempo.

## Para qué
Simular cambios en frecuencias alélicas bajo selección, calcular diferenciación poblacional (FST), modelar dinámicas depredador-presa y crecimiento logístico, visualizar paisajes de aptitud evolutiva, y modelar umbrales epidémicos (R0).

## Funciones principales
- `allele_freq_t(p0, s, t)` — Frecuencia alélica bajo selección en tiempo t
- `genetic_load(W_mean, W_max)` — Carga genética
- `drift_variance(p, N)` — Varianza de deriva genética
- `effective_pop_size(Nf, Nm)` — Tamaño efectivo (Wright)
- `fst_index(Ht, Hs)` — Índice de fijación FST
- `relative_fitness(W, W_max)` — Aptitud relativa
- `selection_coeff(W, W_max)` — Coeficiente de selección s
- `net_reproductive_rate(l0,m0, l1,m1, l2,m2)` — Tasa reproductiva neta R0
- `intrinsic_growth_rate(R0, T_gen)` — Tasa intrínseca de crecimiento
- `logistic_N(N0, K, r, t)` — Crecimiento logístico poblacional
- `lv_growth_rate(r, N, K, N_other, α)` — Lotka-Volterra interespecífica
- `coexistence_possible(K1, K2, α12, α21)` — Condición de coexistencia
- `R0_epidemic(β, N, γ)` — Número básico de reproducción
- `sir_S(S, I, N, β)`, `sir_I(S, I, N, β, γ)`, `sir_R(I, γ)` — Tasas SIR

## SDFs definidas
- `fitness_landscape(ruggedness, scale)` — Paisaje de aptitud (fbm 2D)
- `ecological_niche(cx, cz, wx, wz, height)` — Nicho ecológico gaussiano

## Notas
- La capa en el header (5.1) difiere de la numeración del sistema general (6.3); el contenido pertenece a la capa 6 de Biología.
- La función `fitness_landscape` usa `fbm2d_4` de la librería de ruido para generar rugosidad.
- Los modelos SIR son determinísticos; no incluyen estocasticidad demográfica.
