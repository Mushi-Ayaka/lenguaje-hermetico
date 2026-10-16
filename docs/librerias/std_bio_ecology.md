# std_bio_ecology.hm

**Capa:** 6.7 — Ecología de Poblaciones y Comunidades
**Versión:** 0.2
**Fuente:** Lotka-Volterra Avanzado, Índice de Shannon, Metapoblaciones
**Dependencias:** std_math_core.hm, std_math_probability.hm
**Fecha:** 6 de junio de 2026

## Cómo
Modela dinámicas depredador-presa clásicas (Lotka-Volterra), metapoblaciones (modelo de Levins con colonización y extinción), diversidad de comunidades (riqueza de Margalef, dominancia de Simpson) y distribución espacial de parches. La SDF representa paisajes fragmentados como malla de parches.

## Por qué
La ecología estudia las interacciones entre organismos y su entorno. Comprender la dinámica de poblaciones, la estructura de comunidades y la conectividad espacial es esencial para la conservación y el modelado de ecosistemas.

## Para qué
Simular ciclos depredador-presa, calcular proporción de equilibrio de parches ocupados en metapoblaciones, medir diversidad y dominancia en comunidades, y generar paisajes de parches para visualización espacial.

## Funciones principales
- `lv_prey_dv(N, P, α, β)` — Tasa de cambio de presas (Lotka-Volterra)
- `lv_predator_dv(N, P, δ, γ)` — Tasa de cambio de depredadores
- `levins_metapopulation_dp(p, c, e)` — Cambio en proporción de parches ocupados
- `levins_equilibrium(c, e)` — Proporción de equilibrio p*
- `margalef_richness(S, N)` — Índice de riqueza de Margalef
- `simpson_dominance3(p1, p2, p3)` — Dominancia de Simpson (3 especies)

## SDFs definidas
- `patch_landscape(patch_size, sparsity)` — Paisaje fragmentado en cuadrícula

## Notas
- El modelo de Levins asume una metapoblación infinita de parches idénticos.
- El índice de Simpson se implementa para 3 especies; para más especies extender la suma.
- `patch_landscape` genera una malla de parches cuadrados usando la función `frac`.
