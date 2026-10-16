# Índice de Librerías — Lenguaje Hermético

**Total:** 55 librerías distribuidas en 8 capas ontológicas + 2 capas puente.

---

## Capa 1 — Matemáticas (6)

| # | Archivo | Título | Dependencias |
|---|---------|--------|-------------|
| 1 | `std_math_core.md` | Base Matemática | Ninguna |
| 2 | `std_math_algebra.md` | Álgebra Abstracta y Geometría Diferencial | `core` |
| 3 | `std_math_calculus.md` | Cálculo Diferencial e Integral | `core` |
| 4 | `std_math_noise.md` | Ruido Procedural | `core` |
| 5 | `std_math_dynamics.md` | Dinámica Abstracta y Sistemas Caóticos | `core`, `algebra`, `calculus` |
| 6 | `std_math_probability.md` | Probabilidad y Estadística | `core` |

## Capa 2 — Física (10)

| # | Archivo | Título | Dependencias |
|---|---------|--------|-------------|
| 7 | `std_phys_kinematics.md` | Mecánica Clásica | `core`, `algebra`, `calculus` |
| 8 | `std_phys_thermo.md` | Termodinámica | `core`, `calculus` |
| 9 | `std_phys_statmech.md` | Mecánica Estadística | `core`, `calculus` |
| 10 | `std_phys_quantum.md` | Mecánica Cuántica | `core`, `algebra`, `calculus` |
| 11 | `std_phys_qed.md` | Electrodinámica Cuántica | `core`, `algebra`, `calculus` |
| 12 | `std_phys_qcd.md` | Cromodinámica Cuántica | `core`, `algebra`, `calculus` |
| 13 | `std_phys_qft.md` | Teoría Cuántica de Campos | `core`, `algebra`, `calculus` |
| 14 | `std_phys_relativity.md` | Relatividad | `core`, `algebra`, `calculus` |
| 15 | `std_phys_condensed.md` | Materia Condensada | `core`, `algebra`, `calculus`, `quantum` |
| 16 | `std_phys_sdf.md` | SDFs Físicas | `core`, `algebra`, `calculus` |

## Puente Física → Química (7)

| # | Archivo | Título | Dependencias |
|---|---------|--------|-------------|
| 17 | `std_bridge_atomic.md` | Estructura Atómica | `core`, `quantum` |
| 18 | `std_bridge_physchem.md` | Fisicoquímica | `core`, `algebra`, `calculus` |
| 19 | `std_bridge_quantchem.md` | Química Cuántica | `core`, `algebra`, `calculus`, `quantum` |
| 20 | `std_bridge_spectral.md` | Espectroscopía | `core`, `algebra`, `calculus`, `quantum` |
| 21 | `std_bridge_electrochem.md` | Electroquímica | `core`, `algebra`, `calculus` |
| 22 | `std_bridge_kinetics.md` | Cinética Química | `core`, `algebra`, `calculus` |
| 23 | `std_bridge_statmech_mol.md` | Termo Estadística Molecular | `core`, `calculus`, `statmech` |

## Capa 3–4 — Química (11)

| # | Archivo | Título | Dependencias |
|---|---------|--------|-------------|
| 24 | `std_chem_atoms.md` | Estructura Atómica y Tabla Periódica | `core`, `quantum` |
| 25 | `std_chem_bonds.md` | Enlaces Químicos | `core`, `algebra`, `quantum` |
| 26 | `std_chem_reactions.md` | Reacciones Químicas | `core`, `algebra`, `calculus` |
| 27 | `std_chem_organic.md` | Química Orgánica | `core`, `algebra` |
| 28 | `std_chem_inorganic.md` | Química Inorgánica | `core`, `algebra` |
| 29 | `std_chem_polymer.md` | Polímeros y Materia Blanda | `core`, `algebra`, `calculus` |
| 30 | `std_chem_crystal.md` | Cristalografía | `core`, `algebra` |
| 31 | `std_chem_optics.md` | Óptica y Materiales | `core`, `algebra` |
| 32 | `std_chem_coordination.md` | Química de Coordinación | `core`, `algebra`, `quantum` |
| 33 | `std_chem_supramolecular.md` | Química Supramolecular | `core`, `algebra` |
| 34 | `std_chem_theoretical.md` | Química Teórica | `core`, `algebra`, `calculus`, `quantum` |

## Puente Química → Biología (6)

| # | Archivo | Título | Dependencias |
|---|---------|--------|-------------|
| 35 | `std_bridge_biochem.md` | Bioquímica | `core`, `algebra`, `atoms`, `bonds`, `reactions` |
| 36 | `std_bridge_biophysics.md` | Biofísica | `core`, `algebra`, `calculus` |
| 37 | `std_bridge_chembio.md` | Química Biológica | `core`, `algebra` |
| 38 | `std_bridge_molbio.md` | Biología Molecular | `core`, `algebra`, `calculus`, `noise` |
| 39 | `std_bridge_genomics.md` | Genómica | `core`, `algebra`, `calculus` |
| 40 | `std_bridge_enzymology.md` | Enzimología | `core`, `algebra`, `calculus` |

## Capa 6 — Biología (8)

| # | Archivo | Título | Dependencias |
|---|---------|--------|-------------|
| 41 | `std_bio_cell.md` | Biología Celular | `core`, `noise` |
| 42 | `std_bio_genetics.md` | Genética y Epigenética | `core` |
| 43 | `std_bio_evolution.md` | Evolución | `core` |
| 44 | `std_bio_ecology.md` | Ecología de Poblaciones | `core` |
| 45 | `std_bio_neuro.md` | Neurobiología | `core` |
| 46 | `std_bio_physiology.md` | Fisiología | `core` |
| 47 | `std_bio_systems.md` | Biología de Sistemas | `core` |
| 48 | `std_bio_development.md` | Desarrollo y Morfogénesis | `core`, `noise` |

## Capa 7 — Geociencias (4)

| # | Archivo | Título | Dependencias |
|---|---------|--------|-------------|
| 49 | `std_geo_atmosphere.md` | Atmósfera | `core`, `algebra`, `calculus`, `noise` |
| 50 | `std_geo_landscape.md` | Paisajes | `core`, `algebra`, `noise` |
| 51 | `std_geo_ocean.md` | Océanos | `core`, `algebra`, `calculus`, `noise` |
| 52 | `std_geo_terrain.md` | Terrenos | `core`, `noise` |

## Capa 8 — Astrofísica (3)

| # | Archivo | Título | Dependencias |
|---|---------|--------|-------------|
| 53 | `std_astro_mechanics.md` | Mecánica Celeste | `core`, `algebra` |
| 54 | `std_astro_stellar.md` | Física Estelar | `core`, `calculus` |
| 55 | `std_astro_cosmology.md` | Cosmología | `core`, `calculus`, `relativity` |

---

## Mapa de dependencias

```
std_math_core (raíz)
├── std_math_algebra
│   ├── std_math_calculus
│   │   ├── std_math_dynamics
│   │   ├── std_phys_* (todas)
│   │   └── ...
│   ├── std_math_noise
│   │   ├── std_bio_cell
│   │   ├── std_bio_development
│   │   ├── std_geo_*
│   │   └── std_bridge_molbio
│   ├── std_math_probability
│   ├── std_bridge_* (varias)
│   └── std_chem_* (varias)
├── std_phys_quantum
│   ├── std_phys_condensed
│   ├── std_bridge_atomic
│   ├── std_bridge_quantchem
│   ├── std_bridge_spectral
│   ├── std_chem_atoms
│   ├── std_chem_bonds
│   ├── std_chem_coordination
│   ├── std_chem_theoretical
│   └── ...
├── std_phys_statmech
│   └── std_bridge_statmech_mol
├── std_phys_relativity
│   └── std_astro_cosmology
└── std_chem_* → std_bridge_biochem → std_bridge_molbio → ...
```

**Nota:** `std_math_core.hm` es la única librería sin dependencias. Todas las demás importan directa o indirectamente de ella.
