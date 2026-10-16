# std_phys_sdf.hm

**Capa:** 2.10 — Física Fundamental (SDFs Científicas)
**Versión:** 1.0
**Fuente:** Física clásica y cuántica, Navier-Stokes, Maxwell
**Dependencias:** std_math_core.hm, std_math_algebra.hm, std_math_calculus.hm
**Fecha:** 6 de junio de 2026

## Cómo
Define las constantes físicas fundamentales normalizadas (GRAVITY_EARTH, SPEED_OF_LIGHT, PLANCK, BOLTZMANN, etc.) y proporciona funciones físicas puras agrupadas por dominio: dinámica de fluidos (Navier-Stokes simplificado, Bernoulli, ondas de gravedad, vórtices), termodinámica (difusión de calor, Fourier, Gibbs), óptica (Snell, Fresnel, Cauchy, lentes), electromagnetismo (campo eléctrico, dipolo magnético, Ampère), ondas (estacionarias, membrana), mecánica cuántica (QHO, hidrógeno 1s, túnel WKB), relatividad (Lorentz, intervalo) y mecánica estadística (Maxwell-Boltzmann, Fermi-Dirac, Bose-Einstein). Además expone ~12 SDFs compuestas listas para usar en escenas.

## Por qué
Es la librería física más transversal del Lenguaje Hermético. Centraliza constantes y funciones que el resto de librerías de la Capa 2 importan, y ofrece formas físicas ya empaquetadas como SDFs para uso directo en escenas.

## Para qué
- Proveer constantes físicas normalizadas a toda la capa física
- Modelar fluidos, ondas, campos electromagnéticos y óptica
- Usar SDFs prehechas (lentes, membranas, vórtices, superficies equipotenciales) en escenas

## Constantes definidas

| Constante | Valor | Descripción |
|---|---|---|
| `GRAVITY_EARTH` | 9.80665 | Gravedad terrestre (m/s²) |
| `SPEED_OF_LIGHT` | 1.0 | Normalizado (c = 1) |
| `PLANCK` | 1.0 | ℏ normalizado |
| `BOLTZMANN` | 1.0 | k_B normalizado |
| `ELECTRON_CHARGE` | 1.0 | e normalizado |
| `VACUUM_PERMITTIVITY` | 1.0 | ε₀ normalizado |
| `STEFAN_BOLTZMANN` | 5.670374419e-8 | W/(m²K⁴) |

## Funciones principales

| Dominio | Funciones |
|---|---|
| **Fluidos** | `fluid_velocity_x/y/z`, `fluid_pressure`, `gravity_wave`, `fluid_vortex`, `breaking_wave` |
| **Termodinámica** | `temperature_field`, `heat_conduction`, `thermal_emissivity`, `gibbs_energy` |
| **Óptica** | `snell_sin_refr`, `fresnel_schlick`, `fresnel_F0`, `cauchy_dispersion`, `spherical_lens_sdf`, `parabolic_lens_sdf` |
| **Electromagnetismo** | `electric_field_magnitude`, `magnetic_dipole_r`, `electric_potential`, `ampere_field` |
| **Ondas** | `standing_wave3d`, `vibrating_membrane`, `wave_with_current` |
| **Cuántica** | `qho_psi0`, `hydrogen_1s`, `tunnel_transmission` |
| **Relatividad** | `lorentz_factor`, `length_contraction`, `time_dilation`, `spacetime_interval` |
| **Estadística** | `maxwell_boltzmann_speed`, `fermi_dirac`, `bose_einstein` |

## SDFs definidas

| SDF | Descripción |
|---|---|
| `gravity_wave_surface(amp, k, g, t_anim)` | Superficie de onda gravitatoria |
| `breaking_wave_surface(amp, k, depth, t_anim)` | Ola rompiente con armónico 2 |
| `spherical_lens(R)` | Lente esférica (menisco) |
| `parabolic_lens(focal_len)` | Lente parabólica (sin aberración cromática) |
| `electric_equipotential(q1, q2, V_iso)` | Superficie equipotencial de dipolo eléctrico |
| `fluid_vortex_shape(radius, strength, t_anim)` | Vórtice fluídico |
| `vibrating_membrane_shape(amp, ω, t_anim)` | Membrana vibrante (modo 1,1) |
| `magnetic_field_isosurface(mx, my, mz, B_iso)` | Isosuperficie de campo dipolar magnético |
| `standing_wave_volume(amp, kx, ky, kz, ω, t_anim)` | Onda estacionaria 3D |
| `isothermal_surface(T_hot, T_cold, L, T_iso)` | Superficie isotérmica |

## Notas
- Es la librería de más alto nivel dentro de la Capa 2. Su versión es 1.0 (madura), a diferencia del resto (0.2).
- La mayoría de constantes están normalizadas a 1 para simplificar cálculos; `STEFAN_BOLTZMANN` y `GRAVITY_EARTH` mantienen valores físicos reales.
- Las SDFs aquí definidas son composiciones directas de las funciones físicas puras, listas para ser instanciadas en escenas.
