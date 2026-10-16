# std_chem_crystal.hm

**Capa:** 3.5 — Cristalografía y Estado Sólido Molecular
**Versión:** 0.2
**Fuente:** Bragg, Bravais, International Tables of Crystallography
**Dependencias:** std_math_core.hm, std_math_algebra.hm, std_phys_condensed.hm
**Fecha:** 6 de junio de 2026

## Cómo
Define redes de Bravais (cúbica simple, hexagonal), ley de Bragg, espaciado de planos (hkl), factor de estructura FCC con reglas de extinción, y SDFs de celdas unitarias y estructuras cristalinas (celda cúbica, hexagonal, NaCl, zeolita porosa, diamante cúbico).

## Por qué
La cristalografía es esencial para entender la estructura de sólidos. Proporciona las herramientas para modelar periodicidad cristalina y difracción.

## Para qué
- Generar coordenadas de redes cristalinas (SC, hexagonal)
- Calcular ángulos de Bragg y espaciado de planos
- Evaluar factores de estructura (FCC) con reglas de extinción
- Visualizar celdas unitarias y estructuras cristalinas (NaCl, diamante, zeolita)

## Funciones principales
| Función | Descripción |
|---|---|
| `sc_lattice_{x,y,z}(i, a)` | Red cúbica simple |
| `hex_lattice_{x,y,z}(n₁, n₂, n₃, a, c)` | Red hexagonal |
| `bragg_condition(n, λ, d)` | Ángulo de Bragg |
| `d_spacing_cubic(a, h, k, l)` | Espaciado de planos (hkl) |
| `fcc_structure_factor(h, k, l)` | Factor de estructura FCC |

## SDFs definidas
| SDF | Descripción |
|---|---|
| `simple_cubic_cell(a)` | Celda unitaria cúbica simple |
| `hexagonal_cell(a, c)` | Celda hexagonal |
| `nacl_crystal(a, r_ion)` | Iones en red NaCl |
| `zeolite_pore(a, r_pore)` | Red porosa de zeolita |
| `diamond_cubic(a, r_atom)` | Estructura de diamante |

## Notas
El factor de estructura FCC usa cosenos para simplificar la exponencial compleja. Las SDFs cristalinas usan `round()` y `frac()` para lograr periodicidad.
