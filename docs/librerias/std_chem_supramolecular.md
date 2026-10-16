# std_chem_supramolecular.hm

**Capa:** Química (4.4 — Química Supramolecular)
**Versión:** 1.0
**Fuente:** "Supramolecular Chemistry" by Steed & Atwood
**Dependencias:** std_math_core.hm, std_math_algebra.hm, std_chem_organic.hm, std_chem_coordination.hm, std_bridge_biochem.hm
**Fecha:** 6 de junio de 2026

## Cómo
Implementa modelos de química supramolecular: reconocimiento molecular (energía de enlace huésped-hospedador, constante de asociación, selectividad, complementariedad geométrica, energía de preorganización), autoensamblaje (criterio espontáneo, tasa de ensamblaje, estabilidad de arquitectura, cooperatividad, defectos), máquinas moleculares (eficiencia de motor, velocidad de rotación, ciclo de trabajo, switches, actuadores), y SDFs de arquitecturas supramoleculares (micela, vesícula liposomal, catenano, rotaxano, cavitando, autoensamblaje hexagonal).

## Por qué
La química supramolecular va más allá de la molécula individual hacia sistemas complejos autoensamblados. Es la base de la nanotecnología molecular y los materiales inteligentes.

## Para qué
- Modelar energías de interacción huésped-hospedador
- Calcular constantes de asociación y selectividad
- Simular criterios de autoensamblaje espontáneo
- Modelar máquinas moleculares (motores, switches, actuadores)
- Visualizar arquitecturas: micelas, vesículas, catenanos, rotaxanos, cavitandos, autoensamblajes

## Funciones principales
| Función | Descripción |
|---|---|
| `host_guest_binding_energy(surfaces, H_bonds, VdW, elec)` | Energía de enlace huésped-hospedador (kJ/mol) |
| `association_constant(ΔG, T)` | Constante de asociación K_a |
| `recognition_selectivity(ΔG_target, ΔG_competitor)` | Selectividad de reconocimiento |
| `geometric_complementarity(cavity, guest, flexibility)` | Complementariedad geométrica |
| `self_assembly_criterion(ΔG, TΔS, T)` | Criterio de autoensamblaje (ΔG < 0) |
| `cooperativity_factor(E_individual, E_total)` | Factor de cooperatividad |
| `molecular_motor_efficiency(work, input)` | Eficiencia de motor molecular |
| `molecular_switch_state(input, threshold, state)` | Estado de switch molecular |
| `critical_micelle_concentration(tail, head, T)` | CMC de surfactantes |
| `liquid_crystal_phase(aspect_ratio, conc, T)` | Fase de cristal líquido |

## SDFs definidas
| SDF | Descripción |
|---|---|
| `spherical_micelle_structure(N, length, threshold)` | Micela esférica (cabezas + núcleo) |
| `vesicle_liposome_structure(r, bilayer, threshold)` | Vesícula liposomal bicapa |
| `catenane_interlocked_rings(r, thickness, twist, threshold)` | Catenano (anillos entrelazados) |
| `rotaxane_structure(axle_len, ring_r, ring_t, stopper_r, threshold)` | Rotaxano (anillo en eje) |
| `host_receptor_cavitand(cavity, rim, threshold)` | Receptor cavitando con huésped |
| `hexagonal_self_assembly(tile, strength, defects, threshold)` | Autoensamblaje hexagonal |

## Notas
V1.0. Los catenanos y rotaxanos son MEIs (Molecular Entanglements/Interlocks). Las SDFs usan `sphere`, `cylinder`, `torus`, `union` y `difference`. La variable `w` se usa para animación en rotaxano.
