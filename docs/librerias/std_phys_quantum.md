# std_phys_quantum.hm

**Capa:** 2.4 — Física Cuántica y Mecánica de Matrices
**Versión:** 0.2
**Fuente:** Schrödinger, Heisenberg, Dirac
**Dependencias:** std_math_core.hm, std_math_algebra.hm, std_math_calculus.hm
**Fecha:** 6 de junio de 2026

## Cómo
Implementa soluciones analíticas de la ecuación de Schrödinger 1D (pozo infinito, oscilador armónico, escalón de potencial), orbitales atómicos del hidrógeno (1s, 2s, 2pz), paquetes de ondas gaussianos y la relación de incertidumbre.

## Por qué
La mecánica cuántica describe la naturaleza a escalas atómicas. Esta librería permite visualizar funciones de onda, densidades de probabilidad y orbitales atómicos como geometrías en el Lenguaje Hermético.

## Para qué
- Renderizar nubes de probabilidad orbital (1s, 2pz) como SDFs
- Visualizar paquetes de ondas y su esparcimiento temporal
- Modelar túneles cuánticos y coeficientes de reflexión/transmisión

## Funciones principales

| Función | Descripción |
|---|---|
| `inf_well_psi(n, L, x)` | Función de onda del pozo infinito: √(2/L)·sin(nπx/L) |
| `inf_well_prob(n, L, x)` | Densidad de probabilidad |ψ|² |
| `inf_well_energy(n, m, L)` | Energía del pozo infinito |
| `qho_psi0_1d(m, ω, x)` | Estado base del oscilador armónico (gaussiana) |
| `qho_energy(n, ω)` | Energía del oscilador: ℏω(n + ½) |
| `potential_step_R(k₁, k₂)` | Coeficiente de reflexión en escalón |
| `potential_step_T(k₁, k₂)` | Coeficiente de transmisión en escalón |
| `bohr_radius(m_e, e, ε₀)` | Radio de Bohr: a₀ |
| `orbital_1s(r, a₀)` | Orbital 1s: e^(−r/a₀) |
| `prob_density_1s(r, a₀)` | Densidad de probabilidad 1s |
| `radial_dist_1s(r, a₀)` | Distribución radial: 4πr²|ψ|² |
| `orbital_2s(r, a₀)` | Orbital 2s: (2 − r/a₀)·e^(−r/2a₀) |
| `orbital_2pz(r, θ, a₀)` | Orbital 2pz: (r/a₀)·cosθ·e^(−r/2a₀) |
| `wave_packet(x, t, k₀, v_g, σ_x)` | Paquete de ondas gaussiano móvil |
| `uncertainty_limit(σ_x)` | Límite de incertidumbre: ℏ/(2σ_x) |
| `wave_packet_spreading(σ₀, m, t)` | Esparcimiento del paquete: σ₀√(1 + (ℏt/(2mσ₀²))²) |

## SDFs definidas

| SDF | Descripción |
|---|---|
| `orbital_1s_cloud(a₀, prob_threshold)` | Nube de probabilidad 1s (isosuperficie de |ψ|²) |
| `orbital_2pz_cloud(a₀, prob_threshold)` | Nube de probabilidad 2pz (lóbulos polares) |
| `moving_wave_packet(k₀, v_g, σ_x, threshold, t_anim)` | Paquete de ondas móvil en 3D |

## Notas
- Los orbitales 2s y 2pz son aproximaciones no normalizadas.
- `PLANCK` está normalizado (=1) desde `std_phys_sdf.hm`.
- Las SDFs cuánticas permiten visualizar densidades de probabilidad como volúmenes implícitos.
