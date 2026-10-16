# std_phys_statmech.hm

**Capa:** 2.3 — Mecánica Estadística
**Versión:** 0.2
**Fuente:** Boltzmann, Gibbs, Maxwell — mecánica estadística
**Dependencias:** std_math_core.hm, std_phys_sdf.hm
**Fecha:** 6 de junio de 2026

## Cómo
Implementa el factor de Boltzmann, función de partición, probabilidad canónica, energía promedio y capacidad calorífica para sistemas de 2 estados; distribuciones de velocidad de Maxwell; entropía de Boltzmann y Gibbs; energía libre de Helmholtz; fluctuaciones; parámetro de orden de Ising y temperatura crítica 2D.

## Por qué
La mecánica estadística conecta el comportamiento microscópico con las propiedades macroscópicas. En el Lenguaje Hermético permite generar campos de probabilidad y temperatura basados en primeros principios estadísticos.

## Para qué
- Visualizar distribuciones de velocidad molecular como campos escalares
- Modelar transiciones de fase y fluctuaciones térmicas
- Generar SDFs basadas en densidad de probabilidad

## Funciones principales

| Función | Descripción |
|---|---|
| `boltzmann_factor(E, T)` | Factor de Boltzmann: e^(−E/kT) |
| `partition_2state(E₁, E₂, T)` | Función de partición de 2 estados |
| `canonical_prob(E, Z, T)` | Probabilidad canónica: e^(−E/kT)/Z |
| `mean_energy_2state(E₁, E₂, T)` | Energía promedio ⟨E⟩ |
| `heat_capacity_2state(E₁, E₂, T)` | Capacidad calorífica C_V |
| `maxwell_peak_speed(m, T)` | Velocidad más probable: √(2kT/m) |
| `maxwell_mean_speed(m, T)` | Velocidad media: √(8kT/πm) |
| `maxwell_rms_speed(m, T)` | Velocidad RMS: √(3kT/m) |
| `boltzmann_entropy(Ω)` | Entropía: S = k_B·ln(Ω) |
| `helmholtz_free_energy(Z, T)` | Energía libre: F = −kT·ln(Z) |
| `gibbs_entropy2(p₁, p₂)` | Entropía de Gibbs: −k_B·Σp_i·ln(p_i) |
| `relative_fluctuation(N)` | Fluctuación relativa: 1/√N |
| `ising_order_param(J, h, T, m_prev)` | Parámetro de orden de Ising (campo medio) |
| `ising_critical_temp(J)` | Tc de Ising 2D: 2J/ln(1+√2) |

## SDFs definidas

| SDF | Descripción |
|---|---|
| `maxwell_boltzmann_field(m, T, iso_level)` | Isosuperficie de distribución de Maxwell-Boltzmann en 3D |
| `thermal_field_surface(E₁, E₂, T_iso)` | Campo de temperatura del ensamble canónico |

## Notas
- Usa `BOLTZMANN` normalizado (=1) de `std_phys_sdf.hm`.
- `select` y `step` se usan en `gibbs_entropy2` para evitar log(0).
- El modelo de Ising es de campo medio (ecuación autoconsistente simplificada para una iteración).
