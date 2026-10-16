# std_phys_qed.hm

**Capa:** 2.5 — Electrodinámica Cuántica (QED)
**Versión:** 0.2
**Fuente:** Feynman, Schwinger, Tomonaga
**Dependencias:** std_math_core.hm, std_phys_sdf.hm, std_phys_relativity.hm
**Fecha:** 6 de junio de 2026

## Cómo
Implementa la constante de estructura fina, secciones eficaces de dispersión (Møller, Bhabha/annihilación, Thomson, Compton), correcciones cuánticas de la polarización del vacío (potencial de Uehling) y el momento magnético anómalo del electrón.

## Por qué
QED es la teoría cuántica de campos más precisa de la física. Esta librería acerca sus resultados fundamentales al Lenguaje Hermético para visualización de procesos de scattering y correcciones radiativas.

## Para qué
- Visualizar diagramas de Feynman como SDFs (vértice primitivo)
- Modelar secciones eficaces de dispersión electromagnética
- Simular correcciones cuánticas al potencial de Coulomb

## Funciones principales

| Función | Descripción |
|---|---|
| `moller_cross_section_nr(v, θ, m)` | Dispersión e⁻e⁻ (Møller, no relativista) |
| `annihilation_cross_section(s)` | Aniquilación e⁺e⁻ → μ⁺μ⁻ (alta energía) |
| `thomson_cross_section(r_e)` | Dispersión Compton (Thomson, baja energía) |
| `compton_shift(θ, m)` | Cambio de longitud de onda Compton: Δλ = (h/mc)(1−cosθ) |
| `uehling_potential(q, r, m_e)` | Potencial de Uehling (polarización del vacío) |
| `anomalous_magnetic_moment()` | Momento magnético anómalo: g/2 = 1 + α/2π |

## SDFs definidas

| SDF | Descripción |
|---|---|
| `feynman_vertex(sc_val)` | Vértice de Feynman primitivo (intersección difusa probabilística) |

## Notas
- `ALPHA_FINE` = 0.0072973525693 (constante de estructura fina ≈ 1/137).
- Las secciones eficaces son aproximaciones en nivel árbol (tree-level).
- La función de Uehling es una aproximación para r ≫ 1/m_e.
- `SPEED_OF_LIGHT` y `PLANCK` se importan vía `std_phys_relativity.hm` y `std_math_core.hm`.
