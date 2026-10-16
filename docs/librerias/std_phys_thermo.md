# std_phys_thermo.hm

**Capa:** 2.2 — Termodinámica de Sistemas Fuera del Equilibrio
**Versión:** 0.2
**Fuente:** Fourier, Clausius, Onsager, Prigogine
**Dependencias:** std_math_core.hm, std_phys_sdf.hm, std_phys_statmech.hm
**Fecha:** 6 de junio de 2026

## Cómo
Implementa las cuatro leyes de la termodinámica, conducción y transporte de calor (Fourier, Newton, Stefan-Boltzmann), ciclos termodinámicos (Carnot), fenómenos disipativos (Onsager, Prigogine) y ecuaciones de estado (gas ideal, Van der Waals).

## Por qué
La termodinámica rige los intercambios de energía en todo sistema físico. Al llevarla al Lenguaje Hermético, se pueden simular gradientes térmicos, estructuras disipativas y campos de temperatura como geometrías.

## Para qué
- Visualizar campos de temperatura y flujos de calor como SDFs
- Modelar sistemas fuera del equilibrio y estructuras disipativas
- Simular ciclos termodinámicos y eficiencias energéticas

## Funciones principales

| Función | Descripción |
|---|---|
| `thermal_equilibrium(T1, T2)` | Diferencia de temperatura (equilibrio si ≈ 0) |
| `internal_energy_change(Q, W)` | Primera ley: ΔU = Q − W |
| `entropy_change(Q, T)` | Segunda ley: dS = δQ/T |
| `entropy_near_absolute_zero(T, S₀)` | Tercera ley: S → 0 cuando T → 0 |
| `heat_flux(κ, dT/dx)` | Ley de Fourier: q = −κ∇T |
| `thermal_diffusivity(κ, ρ, Cp)` | Difusividad térmica: α = κ/(ρCp) |
| `heat_eq_1d(T_init, T_bc, κ, ρ, Cp, L, x, t)` | Solución analítica 1D de la ecuación de calor |
| `newton_cooling(T_obj, T_amb, h, A)` | Enfriamiento por convección |
| `radiation_power(ε, A, T_obj, T_amb)` | Radiación Stefan-Boltzmann |
| `carnot_efficiency(T_cold, T_hot)` | Eficiencia de Carnot: 1 − T_c/T_h |
| `carnot_work(T_cold, T_hot, Q_hot)` | Trabajo del ciclo de Carnot |
| `entropy_production(J, X)` | Producción de entropía: σ = J·X |
| `onsager_flux(L₁₁, L₁₂, X₁, X₂)` | Flujo acoplado de Onsager |
| `dissipative_osc(c₀, A, ω, γ, t)` | Oscilación disipativa (Prigogine) |
| `ideal_gas_pressure(n, T, V)` | Gas ideal: P = nkT/V |
| `van_der_Waals_P(T, V_m, a, b)` | Gas de Van der Waals |

## SDFs definidas

| SDF | Descripción |
|---|---|
| `temperature_gradient_field(T_hot, T_cold, κ, L)` | Isosuperficie del campo de temperatura 1D |
| `heat_plume(base_temp, ambient_temp, radius)` | Pluma de convección ascendente |

## Notas
- Usa la constante `STEFAN_BOLTZMANN` de `std_phys_sdf.hm` para radiación.
- `BOLTZMANN` se usa en ecuaciones de estado (importada vía `std_phys_statmech.hm`).
- `tanh_approx` se usa para aproximar la tercera ley.
- Las funciones disipativas permiten modelar sistemas abiertos lejos del equilibrio (Prigogine).
