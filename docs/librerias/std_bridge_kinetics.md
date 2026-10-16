# std_bridge_kinetics.hm

**Capa:** Intermediario Física-Química (3.7 - Cinética Química y Termodinámica Química)
**Versión:** 1.0
**Fuente:** "Chemical Kinetics and Dynamics" by Steinfeld, Francisco & Hase
**Dependencias:** std_math_core.hm, std_math_algebra.hm, std_math_calculus.hm, std_phys_statmech.hm, std_bridge_physchem.hm, std_bridge_statmech_mol.hm
**Fecha:** 6 de junio de 2026

## Cómo
Implementa las principales teorías de velocidad de reacción (colisiones, estado de transición TST, RRKM, Marcus para transferencia electrónica), cinética de mecanismos complejos (consecutivo, paralelo, reversible, en cadena, autocatalítico), cinética enzimática avanzada (Michaelis-Menten con inhibición, Hill con cooperatividad, múltiples sustratos), termodinámica de no equilibrio (Onsager, producción de entropía, fluctuación-disipación, respuesta lineal) y sistemas de oscilaciones y patrones (Lotka-Volterra, Brusselator, condición de Turing). Incluye SDFs para superficies de energía potencial, teoría de colisiones, estado de transición, cinética enzimática, oscilaciones BZ, reacción-difusión, catálisis heterogénea, polimerización, autoensamblaje (micelas), reloj químico y gradientes en reactor.

## Por qué
La cinética química revela el camino de la reacción en el tiempo. Esta librería integra las teorías de velocidad con los mecanismos y la termodinámica de procesos irreversibles.

## Para qué
Modelar constantes de velocidad (colisiones, TST, RRKM, Marcus), mecanismos de reacción complejos (consecutivos, paralelos, reversibles, en cadena, autocatalíticos), cinética enzimática con inhibición y cooperatividad, termodinámica irreversible, oscilaciones químicas, patrones de Turing, catálisis heterogénea, polimerización, autoensamblaje y relojes químicos.

## Funciones principales
- **collision_theory_rate(Z, P, E_a, T)** — k = P·Z·exp(-E_a/RT)
- **collision_frequency_theory(σ, μ, [A], [B], T)** — Z_AB = σ·√(8kT/πμ)·N_A·[A][B]
- **transition_state_theory_rate(T, K‡, ΔG‡)** — k_TST = (k_BT/h)·K‡·exp(-ΔG‡/RT)
- **rrmk_rate_constant(E, E₀, ρ(E), Σ(E))** — k(E) = Σ(E-E₀)/[h·ρ(E)]
- **marcus_electron_transfer_rate(V, λ, ΔGº, T)** — k_ET = (2π/h)|V|²·exp(-(ΔGº+λ)²/4λk_BT)/√(4πλk_BT)
- **consecutive_reaction_kinetics(A₀, k₁, k₂, t)** — A → B → C
- **parallel_reaction_kinetics(A₀, k₁, k₂, t)** — A → B, A → C
- **reversible_reaction_kinetics(A₀, B₀, k_f, k_r, t)** — A ⇌ B
- **chain_reaction_kinetics(k_init, k_prop, k_term, t)** — Mecanismo radicalario
- **autocatalytic_kinetics(A₀, B₀, k, t)** — A + B → 2B
- **hill_equation_cooperativity([S], V_max, n, K_d)** — v = V_max·[S]ⁿ/(Kⁿ+[S]ⁿ)
- **multi_substrate_kinetics([A], [B], V_max, K_mA, K_mB, K_iA, mechanism)** — Ordenado/Aleatorio/Ping-pong
- **onsager_relations(forces, L_ij)** — J_i = Σ L_ij·X_j, L_ij = L_ji
- **entropy_production_rate(fluxes, forces)** — σ = Σ J_i·X_i ≥ 0
- **lotka_volterra_equations(x, y, α, β, γ, δ, Δt)** — Depredador-presa
- **brusselator_reaction_diffusion(A, B, k, D, position)** — Reacción-difusión
- **turing_instability_condition(D_ratio, J)** — D_y/D_x > |J_22|/J_11

## SDFs definidas
- **reaction_potential_energy_surface(reaction_coordinate, energy_profile, intensity_threshold)** — PES con trayectoria
- **molecular_collision_theory_visualization(σ, relative_velocity, intensity_threshold)** — Colisiones moleculares
- **transition_state_complex_visualization(reactants, products, ts_geometry, intensity_threshold)** — Complejo activado
- **michaelis_menten_kinetics_visualization(substrate_range, vmax, km, intensity_threshold)** — Curva de saturación
- **chemical_oscillations_bz_pattern(time, rate_constants, intensity_threshold)** — BZ espiral
- **reaction_diffusion_system(D, k, time, intensity_threshold)** — Sistema R-D
- **heterogeneous_catalysis_surface(surface_sites, adsorption_rate, reaction_rate, intensity_threshold)** — Catálisis superficial
- **polymerization_kinetics_visualization(monomer, k_init, k_prop, intensity_threshold)** — Crecimiento de cadenas
- **molecular_self_assembly_micelles(surfactant, CMC, intensity_threshold)** — Micelas por encima de CMC
- **chemical_clock_temporal_oscillations(time, period, amplitude, intensity_threshold)** — Reloj químico pulsante
- **concentration_gradient_reactor(inlet_concentration, k, D, intensity_threshold)** — Reactor de flujo

## Notas
- RRKM y Marcus son formulaciones conceptuales simplificadas.
- Los mecanismos complejos devuelven arreglos de concentraciones [A, B, C] para cada instante.
- La cinética enzimática incluye inhibición (competitiva, no competitiva, acompetitiva) y mecanismos de múltiples sustratos.
- Las SDFs de autoensamblaje y polimerización son representaciones geométricas del proceso.
- La condición de Turing permite explorar formación de patrones por difusión.
