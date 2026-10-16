# std_chem_coordination.hm

**Capa:** Química (4.3 — Química de Coordinación)
**Versión:** 1.0
**Fuente:** "Coordination Chemistry" by Cotton & Wilkinson
**Dependencias:** std_math_core.hm, std_math_algebra.hm, std_bridge_quantchem.hm, std_chem_inorganic.hm, std_phys_condensed.hm
**Fecha:** 6 de junio de 2026

## Cómo
Implementa teoría de complejos de coordinación: clasificación de ligandos, serie espectroquímica, número de coordinación, teoría del campo cristalino (desdoblamiento octaédrico/tetraédrico, energía de estabilización, predicción alto/bajo espín), propiedades magnéticas (momento magnético, susceptibilidad de Curie), espectros electrónicos (transiciones d-d, diagramas Tanabe-Sugano, predicción de color, reglas de selección de Laporte), y SDFs de complejos (octaédrico, tetraédrico, cuadrado plano, campo cristalino visualizado, quelación bidentada).

## Por qué
Los complejos de coordinación son centrales en química inorgánica, catálisis, magnetismo y espectroscopia. Explica por qué los complejos tienen color, magnetismo y geometrías específicas.

## Para qué
- Clasificar ligandos por denticidad, hapticidad y fuerza (serie espectroquímica)
- Predecir geometría por número de coordinación
- Calcular desdoblamiento del campo cristalino (Δ_oct, Δ_tet)
- Determinar configuraciones de alto/bajo espín
- Estimar momento magnético efectivo y susceptibilidad
- Predecir color de complejos por transiciones d-d
- Visualizar complejos y desdoblamiento de orbitales d

## Funciones principales
| Función | Descripción |
|---|---|
| `ligand_classification(donors, hapticity, bite)` | Clasificación de ligandos |
| `ligand_field_strength(type)` | Fuerza de ligando (serie espectroquímica) |
| `coordination_number_geometry(CN, ligands)` | Geometría por número de coordinación |
| `crystal_field_splitting_octahedral(metal, ox, ligands)` | Δ_oct (cm⁻¹) |
| `crystal_field_splitting_tetrahedral(metal, ox, ligands)` | Δ_tet = (4/9)·Δ_oct |
| `crystal_field_stabilization_energy(e⁻, geom, Δ, P)` | EECC |
| `high_spin_low_spin_prediction(Δ, P, e⁻)` | Predicción alto/bajo espín |
| `effective_magnetic_moment(n, spin_only)` | Momento magnético efectivo |
| `dd_transition_energies(d_e⁻, geom, Δ, B)` | Energías de transiciones d-d |
| `complex_color_prediction(λ_abs)` | Color del complejo (complementario) |

## SDFs definidas
| SDF | Descripción |
|---|---|
| `octahedral_complex(metal_pos, r_metal, ligand_pos, r_lig, threshold)` | Complejo octaédrico con orbitales d |
| `tetrahedral_complex(metal_pos, r_metal, dist, threshold)` | Complejo tetraédrico |
| `square_planar_complex(metal_pos, r_metal, dist, threshold)` | Complejo cuadrado plano |
| `crystal_field_splitting_visualization(Δ, e⁻_config, threshold)` | Diagrama de desdoblamiento del campo cristalino |
| `chelation_bidentate_ligand(metal, ligands, bite, threshold)` | Quelación bidentada visualizada |

## Notas
V1.0. Las transiciones d-d están implementadas para configuraciones d², d³ y d⁸ octaédricas. El momento magnético usa aproximación spin-only con factor orbital opcional. Constantes BOHR_MAGNETON, PLANCK y BOLTZMANN específicas del dominio.
