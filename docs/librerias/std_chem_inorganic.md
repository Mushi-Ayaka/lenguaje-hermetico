# std_chem_inorganic.hm

**Capa:** Química (4.1 — Química Inorgánica)
**Versión:** 1.0
**Fuente:** "Inorganic Chemistry" by Shriver & Atkins
**Dependencias:** std_math_core.hm, std_math_algebra.hm, std_bridge_atomic.hm, std_bridge_quantchem.hm, std_chem_crystal.hm
**Fecha:** 6 de junio de 2026

## Cómo
Proporciona un ecosistema completo de química inorgánica: constantes fundamentales, configuración electrónica, tendencias periódicas, tipos de enlace inorgánico (iónico, metálico, covalente), teoría VSEPR con predicción de geometrías, estados de oxidación, compuestos inorgánicos (óxidos, haluros, calcogenuros, oxoaniones), y SDFs estructurales (NaCl, fluorita, semiconductores, vidrio, cerámicos, catalizadores).

## Por qué
Cubre toda la diversidad de la química de elementos no orgánicos: metales, minerales, materiales inorgánicos, semiconductores y catalizadores. Fundamental para ciencia de materiales.

## Para qué
- Calcular configuración electrónica y tendencias periódicas
- Predecir geometrías moleculares VSEPR con distorsión por pares solitarios
- Estimar energías de red iónica, enlace metálico y fuerza covalente
- Generar SDFs de estructuras cristalinas (NaCl, fluorita)
- Modelar materiales inorgánicos (semiconductores, vidrio, cerámicos, catalizadores)

## Funciones principales
| Función | Descripción |
|---|---|
| `electron_configuration(Z)` | Configuración electrónica por capas |
| `atomic_radius_periodic(period, group)` | Radio atómico por tendencia periódica |
| `ionization_energy_periodic(Z, degree)` | Energía de ionización |
| `pauling_electronegativity(Z)` | Electronegatividad de Pauling |
| `ionic_lattice_energy(Z⁺, Z⁻, r⁺, r⁻, M)` | Energía de red iónica |
| `percent_ionic_character(Δχ)` | Carácter iónico de Pauling |
| `vsepr_geometry(pairs, lone)` | Geometría VSEPR por pares electrónicos |
| `common_oxidation_states(group, period, block)` | Estados de oxidación comunes |
| `oxide_character(EN_metal, EN_O)` | Carácter ácido-base de óxidos |

## SDFs definidas
| SDF | Descripción |
|---|---|
| `nacl_crystal_structure(a, threshold)` | Red cristalina de NaCl |
| `vsepr_molecular_geometry(center, bond, lone, threshold)` | Geometría VSEPR 3D |
| `octahedral_coordination_complex(metal, ligands, threshold)` | Complejo octaédrico |
| `metal_oxide_layer(atoms, O_pos, thickness, threshold)` | Capa de óxido metálico |
| `fluorite_crystal_structure(a, threshold)` | Red de fluorita (CaF₂) |
| `inorganic_semiconductor_structure(type, gap, threshold)` | Estructura de semiconductor |
| `inorganic_glass_silica(density, disorder, threshold)` | Vidrio de sílice (SiO₂ amorfo) |
| `ceramic_alumina_structure(grain, porosity, threshold)` | Material cerámico Al₂O₃ |
| `inorganic_catalyst_surface(sites, support, threshold)` | Superficie catalítica |

## Notas
Es la librería más extensa de química. V1.0 con madurez en la interfaz. Usa `custom(expr)` para SDFs programáticas. Los ejemplos comentados muestran uso con nodos completos.
