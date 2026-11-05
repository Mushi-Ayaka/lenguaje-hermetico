# std_bio_development.hm

**Capa:** 6.3 — Biología del Desarrollo y Morfogénesis
**Versión:** 1.0
**Fuente:** "Principles of Development" (Wolpert), "The Chemical Basis of Morphogenesis" (Turing)
**Dependencias:** std_math_core.hm, std_math_calculus.hm, std_math_dynamics.hm, std_bio_cell.hm, std_bio_genetics.hm
**Fecha:** 6 de junio de 2026

## Cómo
Modela patrones de reacción-difusión de Turing (amplitud sinusoidal, patrones de estrías y manchas), gradientes morfogénicos (exponencial, lineal, respuesta umbral sigmoide), desarrollo embrionario (reloj de segmentación, especificación de ejes corporales A-P/D-V, información posicional de Wolpert) y morfogénesis tisular (plegamiento epitelial, invaginación, migración celular). Las SDFs representan embriones tempranos, patrones de segmentación y pliegues tisulares.

## Por qué
El desarrollo embrionario transforma una sola célula en un organismo complejo mediante patrones espacio-temporales de expresión génica. La morfogénesis explica cómo las fuerzas mecánicas y químicas esculpen los tejidos.

## Para qué
Generar patrones periódicos tipo Turing (manchas, estrías) para modelado de pigmentación y morfogénesis, simular gradientes morfogénicos que especifican identidad posicional, modelar el reloj de segmentación (somitas), visualizar etapas del desarrollo embrionario en 3D, y simular plegamiento e invaginación tisular.

## Funciones principales
- `turing_pattern_amplitude(x, λ, A, φ)` — Amplitud de patrón de Turing estacionario
- `stripe_pattern(x, width, spacing, A)` — Patrón de estrías periódicas
- `spot_pattern(x, y, radius, A)` — Mancha gaussiana individual (centro en el origen)
- `spot_lattice(x, y, radius, spacing, A)` — Lattice periódico de manchas gaussianas
- `exponential_gradient(x, C0, λ)` — Gradiente exponencial morfogénico
- `linear_gradient(x, C0, L)` — Gradiente lineal
- `thresholded_gradient_response(C, threshold, k)` — Respuesta sigmoide a gradiente
- `segmentation_clock(t, period, φ, n)` — Reloj de segmentación oscilatorio
- `body_axis_specification(pos, L, anterior, posterior)` — Gradiente de eje corporal
- `positional_information(x, field_size, type)` — Información posicional (Wolpert)
- `epithelial_folding(x, freq, amp, thickness)` — Deformación epitelial
- `invagination_depth(r, strength, angle)` — Invaginación radial
- `cell_migration_trajectory(t, speed, dir, τ)` — Trayectoria de migración celular

## SDFs definidas
- `early_embryo_blastula(radius, cell_count, threshold)` — Embrión temprano (blástula)
- `segmentation_pattern(seg_count, seg_size, threshold)` — Patrón de somitos
- `tissue_folding_pattern(freq, depth, threshold)` — Plegamiento tisular sinusoidal

## Notas
- Los patrones de Turing son estacionarios (no resuelven EDPs de reacción-difusión temporalmente).
- `spot_pattern` es una mancha única; `spot_lattice` repite la mancha con separación `spot_spacing` (vía `frac`). Corregido 2026-10-17: `spot_pattern` tenía una línea `sqrt` inerte (error de sintaxis) y un `spot_spacing` sin usar.
- La constante `DIFFUSION_COEFFICIENT_TYPICAL` (1e-10 m²/s) es un valor de referencia para morfógenos.
- La blástula se modela como esfera con perturbación sinusoidal proporcional al número de células.
