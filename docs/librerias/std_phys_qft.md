# std_phys_qft.hm

**Capa:** 2.7 — Teoría Cuántica de Campos
**Versión:** 0.2
**Fuente:** Klein-Gordon, Dirac, propagadores de Feynman
**Dependencias:** std_math_core.hm, std_phys_sdf.hm, std_phys_relativity.hm
**Fecha:** 6 de junio de 2026

## Cómo
Implementa la ecuación de Klein-Gordon (onda escalar masiva), el potencial de Yukawa, propagador de Feynman en espacio-tipo, energía del vacío (cutoff), efecto Casimir y el mecanismo de Higgs (potencial sombrero mexicano, VEV, masa del Higgs).

## Por qué
QFT unifica la mecánica cuántica con la relatividad especial y describe la creación/aniquilación de partículas. Esta librería permite visualizar campos cuánticos y mecanismos de ruptura de simetría en el Lenguaje Hermético.

## Para qué
- Renderizar campos de Yukawa y potenciales escalares como SDFs
- Visualizar el potencial sombrero mexicano del Higgs
- Simular el efecto Casimir y fluctuaciones del vacío

## Funciones principales

| Función | Descripción |
|---|---|
| `klein_gordon_omega(k, m)` | Relación de dispersión: ω² = k² + m² |
| `klein_gordon_plane_wave_re(k, m, x, t)` | Onda plana KG: cos(kx − ωt) |
| `yukawa_potential(g, m, r)` | Potencial de Yukawa: −g²·e^(−mr)/(4πr) |
| `feynman_propagator_spacelike(m, r)` | Propagador de Feynman asintótico: e^(−mr)/r^(3/2) |
| `vacuum_energy_density(k_max)` | Densidad de energía del vacío: k_max⁴/(16π²) |
| `casimir_pressure(d)` | Presión de Casimir: −π²/(240·d⁴) |
| `mexican_hat_potential(φ, μ, λ)` | Potencial de Higgs: −μ²|φ|² + λ|φ|⁴ |
| `vacuum_expectation_value(μ, λ)` | VEV: v = μ/√λ |
| `higgs_mass(μ)` | Masa del Higgs: m_h = √2·μ |

## SDFs definidas

| SDF | Descripción |
|---|---|
| `yukawa_field(g, m, iso_level)` | Superficie equipotencial de Yukawa |
| `mexican_hat_surface(μ, λ, sc)` | Potencial sombrero mexicano (V vs φ en XZ) |
| `false_vacuum_bubble(R, wall_thickness)` | Burbuja de vacío falso (transición de fase) |

## Notas
- Las unidades usan c = 1 y ℏ = 1 (normalizados).
- El propagador de Feynman es una aproximación asintótica para separaciones espacio-tipo grandes.
- El potencial sombrero mexicano es clave en el mecanismo de Higgs y ruptura espontánea de simetría.
