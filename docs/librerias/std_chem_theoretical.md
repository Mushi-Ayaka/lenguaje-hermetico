# std_chem_theoretical.hm

**Capa:** Química (4.7 — Química Teórica y Computacional)
**Versión:** 1.0
**Fuente:** "Computational Chemistry" by Jensen
**Dependencias:** std_math_core.hm, std_math_algebra.hm, std_math_calculus.hm, std_bridge_quantchem.hm, std_phys_statmech.hm
**Fecha:** 6 de junio de 2026

## Cómo
Implementa métodos de química computacional: funcionales DFT (B3LYP híbrido), energía de correlación MP2, costos computacionales (HF O(N⁴), DFT O(N³), CCSD(T) O(N⁷)), dinámica molecular (Velocity-Verlet, Lennard-Jones, campos de fuerza armónicos, torsión, Ewald electrostático), Monte Carlo (Metropolis, traslación, rotación, muestreo de Gibbs), optimización geométrica (gradiente numérico, Hessiano, convergencia, BFGS), análisis vibracional (frecuencias, intensidad IR, actividad Raman), y SDFs de visualización computacional (superficie de energía potencial, trayectorias MD, muestreo MC, optimización, modos vibracionales, malla DFT).

## Por qué
La química computacional es el laboratorio virtual donde se simulan moléculas. Esta librería conecta la teoría química con métodos numéricos prácticos.

## Para qué
- Estimar costos computacionales de métodos cuánticos (HF, DFT, CCSD(T))
- Simular dinámica molecular con Velocity-Verlet y campos de fuerza
- Muestrear configuraciones con Monte Carlo (Metropolis, Gibbs)
- Optimizar geometrías moleculares (gradiente descendente, BFGS)
- Calcular frecuencias vibracionales, intensidad IR y actividad Raman
- Visualizar PES, trayectorias, histogramas de energía y modos normales

## Funciones principales
| Función | Descripción |
|---|---|
| `b3lyp_exchange_correlation(ρ, ∇ρ, τ)` | Funcional B3LYP híbrido |
| `mp2_energy_computation(ε, integrals, occ, virt)` | Energía de correlación MP2 |
| `hartree_fock_cost(N)` | Costo O(N⁴) de HF |
| `dft_cost(N, grid)` | Costo O(N³ + N·G) de DFT |
| `ccsdt_cost(N)` | Costo O(N⁷) de CCSD(T) |
| `velocity_verlet_step(x, v, a, dt)` | Paso de integrador Velocity-Verlet |
| `lennard_jones_force(r, ε, σ)` | Fuerza de Lennard-Jones |
| `harmonic_bond_energy(r, r₀, k)` | Energía de enlace armónico |
| `torsion_energy(φ, V_n, n, γ)` | Energía de torsión (dihedro) |
| `ewald_electrostatic_energy(charges, pos, α, cutoff)` | Energía electrostática Ewald |
| `metropolis_criterion(ΔE, T)` | Criterio de Metropolis |
| `vibrational_frequencies(H, masses)` | Frecuencias vibracionales normales |
| `infrared_intensity(∂μ/∂Q, mode)` | Intensidad IR |
| `raman_activity(∂α/∂Q, mode)` | Actividad Raman |
| `optimization_converged(‖∇E‖, ΔE, thresh)` | Criterio de convergencia |

## SDFs definidas
| SDF | Descripción |
|---|---|
| `potential_energy_surface_computational(grid, energy_fn, threshold)` | Superficie de energía potencial (PES) |
| `md_trajectory_visualization(pos, vel, forces, steps, threshold)` | Trayectoria de dinámica molecular |
| `monte_carlo_sampling_visualization(rate, histogram, threshold)` | Muestreo Monte Carlo (histograma) |
| `geometry_optimization_pathway(initial, opt, steps, threshold)` | Trayectoria de optimización geométrica |
| `vibrational_mode_animation(geom, modes, freq, amp, threshold)` | Modos vibracionales animados |
| `dft_integration_grid_visualization(nuclei, density, threshold)` | Malla de integración DFT |

## Notas
V1.0. El funcional B3LYP es una aproximación híbrida con coeficientes canónicos (0.20 HF + 0.80 Slater + 0.72 B88 + 0.81 LYP). Las SDFs computacionales son principalmente visualizaciones de conceptos abstractos (PES, trayectorias, histogramas).
