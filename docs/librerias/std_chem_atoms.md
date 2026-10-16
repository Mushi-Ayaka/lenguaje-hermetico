# std_chem_atoms.hm

**Capa:** 3.1 — Estructura Atómica y Tabla Periódica
**Versión:** 0.2
**Fuente:** Bohr, Schrödinger, Aufbau principle
**Dependencias:** std_math_core.hm, std_phys_quantum.hm
**Fecha:** 6 de junio de 2026

## Cómo
Define funciones de propiedades atómicas escaladas (radios de Bohr, energía de ionización, electrones por capa, electronegatividad de Mulliken), potenciales de interacción interatómica (Morse, Lennard-Jones), y SDFs de orbitales atómicos (1s, 2s, 2pz) y superficies de potencial de enlace.

## Por qué
Proporciona los bloques fundamentales para modelar átomos individuales, su estructura electrónica y las interacciones básicas entre pares atómicos. Es la base sobre la que se construyen enlaces químicos y moléculas.

## Para qué
- Calcular propiedades periódicas y atómicas básicas
- Modelar orbitales atómicos como SDFs isosuperficiales
- Representar potenciales de interacción (Morse, Lennard-Jones) como SDFs
- Servir de dependencia para librerías de enlaces y geometría molecular

## Funciones principales
| Función | Descripción |
|---|---|
| `bohr_level_radius(n, a0)` | Radio de Bohr para nivel n |
| `ionization_energy(n, E_ground)` | Energía de ionización hidrogenoide |
| `electrons_in_shell(n)` | Electrones por capa: 2n² |
| `vdw_radius_period(period_row)` | Radio de Van der Waals por período |
| `electronegativity_mulliken(IE, EA)` | Electronegatividad de Mulliken |
| `morse_potential(De, a, r_eq, r)` | Potencial de Morse analítico |
| `morse_vib_freq(a, De, μ)` | Frecuencia vibracional Morse |
| `lennard_jones(ε, σ, r)` | Potencial Lennard-Jones 12-6 |

## SDFs definidas
| SDF | Descripción |
|---|---|
| `atom_orbital_1s(a0, iso)` | Orbital 1s (esférico) |
| `atom_orbital_2s(a0, iso)` | Orbital 2s (toroidal) |
| `atom_orbital_2pz(a0, iso)` | Orbital 2pz (lóbulos polares) |
| `morse_bond_sdf(De, a, r_eq, V_iso)` | Pozo de potencial Morse |
| `lj_surface(ε, σ, V_iso)` | Superficie de repulsión LJ |

## Notas
Usa unidades atómicas escaladas. EPSILON previene divisiones por cero. Orbitales atómicos modelados como isosuperficies de densidad de probabilidad.
