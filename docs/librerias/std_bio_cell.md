# std_bio_cell.hm

**Capa:** 6.1 — Biología Celular
**Versión:** 0.2
**Fuente:** Biología molecular de la célula (Alberts et al.)
**Dependencias:** std_math_core.hm, std_math_noise.hm
**Fecha:** 6 de junio de 2026

## Cómo
Modela matemáticamente procesos celulares fundamentales: transporte de iones a través de membranas (Nernst, Goldman-Hodgkin-Katz, Fick, Michaelis-Menten), generación de potenciales de acción (Hodgkin-Huxley), dinámica del ciclo celular (oscilación de CDK), apoptosis por daño acumulado, y crecimiento poblacional logístico. Las SDFs representan orgánulos y estructuras como geometrías Signed Distance Field.

## Por qué
La célula es la unidad fundamental de la vida. Comprender sus mecanismos eléctricos, mecánicos y de señalización es requisito base para cualquier modelo biológico de nivel superior (tejidos, órganos, organismos).

## Para qué
Simular comportamiento electrofisiológico de membranas, visualizar orgánulos en 3D, modelar proliferación y muerte celular, y servir como cimiento para las capas biológicas superiores (genética, fisiología, desarrollo).

## Funciones principales
- `nernst_potential(R, T, z, [X]o, [X]i)` — Potencial de equilibrio iónico
- `goldman_potential(R, T, PK, PNa, PCl, ...)` — Potencial de membrana multicatión
- `fick_flux(D, dC/dx)` — Flujo difusivo simple
- `active_transport(Jmax, [S], Km)` — Transporte activo tipo Michaelis-Menten
- `hh_alpha_m(V)`, `hh_beta_m(V)`, `hh_alpha_n(V)`, `hh_beta_n(V)`, `hh_alpha_h(V)`, `hh_beta_h(V)` — Compuertas del modelo Hodgkin-Huxley
- `hh_current_Na(gNa, m, h, V, ENa)` — Corriente de sodio HH
- `hh_current_K(gK, n, V, EK)` — Corriente de potasio HH
- `hh_current_leak(gL, V, EL)` — Corriente de fuga HH
- `cdk_activity(t, period, phase)` — Oscilación de quinasa dependiente de ciclina
- `apoptosis_signal(damage, threshold)` — Señal de caspasa (sigmoide)
- `cell_growth_rate(N, r, K, inhibition)` — Crecimiento celular logístico

## SDFs definidas
- `spherical_cell(r_cell)` — Célula esférica
- `cell_nucleus(r_nucleus)` — Núcleo esférico interior
- `cell_membrane(r_cell, thickness)` — Membrana como cáscara esférica
- `mitochondrion(a, b)` — Mitocondria elipsoidal
- `endoplasmic_reticulum(thickness, spacing, noise_scale, noise_amp)` — Retículo endoplásmico con ruido
- `cilia(r_base, length, n_cilia, t_anim)` — Cilios oscilantes periféricos
- `cell_division(r_cell, separation)` — Dos células en división

## Notas
- Usa EPSILON como salvaguarda contra división por cero.
- Las compuertas HH son aproximaciones estáticas (no resuelven EDOs temporalmente).
- Las SDFs permiten composición booleana para construir orgánulos complejos.
- La escala física es arbitraria; ajustar según necesidad de visualización.
