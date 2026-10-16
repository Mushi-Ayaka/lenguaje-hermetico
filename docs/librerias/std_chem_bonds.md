# std_chem_bonds.hm

**Capa:** 3.2 — Enlace Químico y Geometría Molecular
**Versión:** 0.2
**Fuente:** VSEPR, hibridación sp, teoría de bandas
**Dependencias:** std_math_core.hm, std_math_algebra.hm, std_chem_atoms.hm
**Fecha:** 6 de junio de 2026

## Cómo
Define constantes de ángulos de enlace canónicos (lineal, trigonal planar, tetrahedral, bipiramidal), funciones de posiciones tetraédricas, integrales de solapamiento orbital, energías de enlace LCAO (MO theory), momentos dipolares, polarizabilidad, y SDFs de geometrías moleculares (enlace covalente, diatómica, agua, metano).

## Por qué
Extiende los átomos aislados hacia sistemas multiátomo. Captura la geometría molecular predictiva (VSEPR), la teoría de orbitales moleculares (LCAO) y las propiedades de polaridad de enlace.

## Para qué
- Construir geometrías moleculares ideales (tetraédrica, agua, metano)
- Calcular integrales de solapamiento y energías de enlace/antienlace
- Estimar órdenes de enlace (Wiberg) y momentos dipolares
- Generar SDFs de moléculas completas para visualización

## Funciones principales
| Función | Descripción |
|---|---|
| `tetrahedral_{x,y,z}(bond_len, i)` | Coordenadas de vértice tetraédrico |
| `overlap_integral_1s(r, a0)` | Solapamiento de orbitales 1s gaussianos |
| `mo_energy_bonding(α, β, S)` | Energía de enlace LCAO |
| `mo_energy_antibonding(α, β, S)` | Energía de antienlace LCAO |
| `bond_order_approx(n_bond, n_anti)` | Orden de enlace de Wiberg |
| `dipole_moment(q, d)` | Momento dipolar |
| `bond_dipole(EN_A, EN_B, bond_len)` | Dipolo por diferencia de electronegatividad |
| `polarizability_sphere(r_atom)` | Polarizabilidad volumétrica |

## SDFs definidas
| SDF | Descripción |
|---|---|
| `covalent_bond(bond_len, radius)` | Enlace covalente simple (cápsula) |
| `diatomic_molecule(bond_len, r1, r2)` | Molécula diatómica |
| `water_molecule(r_O, r_H, bond_len)` | Molécula de agua (104.5°) |
| `methane_molecule(r_C, r_H, bond_len)` | Molécula de metano (tetraédrica) |

## Notas
Los ángulos están en radianes. Las SDFs usan signed distance functions combinadas con `min`/`max`. Las funciones tetraédricas usan `select`/`step` para distribuir vértices.
