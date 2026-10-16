# std_phys_condensed.hm

**Capa:** 2.9 — Física del Estado Sólido y Materia Condensada
**Versión:** 0.2
**Fuente:** Kittel, Ashcroft & Mermin, Bloch, Landau
**Dependencias:** std_math_core.hm, std_phys_sdf.hm, std_phys_statmech.hm
**Fecha:** 6 de junio de 2026

## Cómo
Implementa funciones de Bloch, potenciales de red cristalina (cúbica simple, FCC, BCC), fonones (acústicos y ópticos), densidad de estados de Debye, calor específico, superconductividad (pares de Cooper, longitud de penetración de London, vórtices de Abrikosov) y óptica de Drude (permitividad, reflectividad).

## Por qué
La materia condensada explica las propiedades electrónicas, vibracionales y ópticas de los sólidos. Esta librería permite modelar cristales, fonones y fenómenos superconductores como geometrías y campos.

## Para qué
- Renderizar celdas unitarias y redes cristalinas como SDFs
- Visualizar vórtices de Abrikosov y zonas de Meissner
- Simular dispersión de fonones y respuesta óptica de metales

## Funciones principales

| Función | Descripción |
|---|---|
| `bloch_envelope(kx, ky, kz)` | Envolvente periódica de Bloch |
| `cubic_lattice_potential(V₀, a)` | Potencial periódico de red cúbica simple |
| `fcc_density(a)` | Densidad de red FCC (superposición de gaussianas) |
| `bcc_density(a)` | Densidad de red BCC |
| `optical_phonon_freq(ω₀, D, k)` | Dispersión de fonones ópticos |
| `acoustic_phonon_freq(v_s, k)` | Dispersión de fonones acústicos: ω = v_s·|k| |
| `debye_dos(ω, ω_D)` | Densidad de estados de Debye: ∝ ω² |
| `debye_heat_capacity(T, T_D)` | Calor específico de Debye |
| `cooper_pair_wave(r, ξ)` | Función de onda del par de Cooper: e^(−r/ξ) |
| `meissner_field(B₀, λ_L, z)` | Penetración de London: B₀·e^(−|z|/λ_L) |
| `abrikosov_vortex(r, ξ, λ_L)` | Perfil de vórtice tipo II |
| `drude_permittivity(ω, ω_p, γ)` | Permitividad de Drude: 1 − ω_p²/(ω² + γ²) |
| `drude_reflectivity(ω, ω_p, γ)` | Reflectividad de Drude |

## SDFs definidas

| SDF | Descripción |
|---|---|
| `cubic_unit_cell(a)` | Celda unitaria cúbica (caja) |
| `fcc_crystal(a, iso_level)` | Red cristalina FCC como campo de densidad |
| `abrikosov_vortex_shape(ξ, λ_L)` | Vórtice de Abrikosov (cilindro con decaimiento) |
| `meissner_zone(λ_L, field_threshold)` | Zona de exclusión de campo magnético |

## Notas
- Las densidades de red (FCC, BCC) se modelan como superposición de gaussianas centradas en los sitios de la red.
- `BOLTZMANN` se usa desde `std_phys_statmech.hm`.
- Los vórtices de Abrikosov son característicos de superconductores tipo II.
