# std_bridge_kinetics.hm

**Capa:** Intermediario Física-Química (3.7 - Cinética Química y Termodinámica Química)
**Versión:** 1.0
**Fuente:** "Chemical Kinetics and Dynamics" by Steinfeld, Francisco & Hase
**Dependencias:** std_math_core.hm, std_bridge_atomic.hm
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
- **transition_state_equilibrium_constant(K‡, q_A, q_B, E₀)** — K‡ = q‡/(q_A·q_B)·exp(-E₀/k_BT) (2 reactantes)
- **rrkm_rate_constant(E, E₀, ρ(E), Σ(E))** — k(E) = Σ(E-E₀)/[h·ρ(E)] (ρ y Σ evaluadas por el llamador)
- **marcus_electron_transfer_rate(V, λ, ΔGº, T)** — k_ET = (2π/h)|V|²·exp(-(ΔGº+λ)²/4λk_BT)/√(4πλk_BT)
- **consecutive_reaction_kinetics(A₀, k₁, k₂, t)** — A → B → C
- **parallel_reaction_kinetics(A₀, k₁, k₂, t)** — A → B, A → C
- **reversible_reaction_kinetics(A₀, B₀, k_f, k_r, t)** — A ⇌ B
- **chain_reaction_kinetics(k_init, k_prop, k_term, t)** — Mecanismo radicalario
- **autocatalytic_kinetics(A₀, B₀, k, t)** — A + B → 2B
- **michaelis_menten_with_inhibition([S], [I], V_max, K_m, K_i, inhibition_type)** — 0=competitiva, 1=no competitiva, 2=acompetitiva, 3=sin inhibición
- **hill_equation_cooperativity([S], V_max, n, K_d)** — v = V_max·[S]ⁿ/(Kⁿ+[S]ⁿ)
- **hill_coefficient_from_data([S]₁, [S]₂, v₁, v₂, V_max)** — n_H por 2 puntos (pendiente log-log)
- **multi_substrate_kinetics([A], [B], V_max, K_mA, K_mB, K_iA, mechanism)** — 0=ordenado, 1=aleatorio, 2=ping-pong
- **onsager_relations(X₁, X₂, L₁₁, L₁₂, L₂₁, L₂₂)** — J_i = L_ij·X_j (sistema 2×2)
- **entropy_production_rate(J₁, J₂, X₁, X₂)** — σ = Σ J_i·X_i ≥ 0 (2 pares)
- **fluctuation_dissipation_relation(T, L, Δt)** — ⟨δJ·δJ⟩ = 2k_BT·L·δ(Δt)
- **linear_response_function(χ, δF, t)** — ⟨δA(t)⟩ = ∫Φ(t-t')·δF(t')dt'
- **lotka_volterra_equations(x, y, α, β, γ, δ, Δt)** — Depredador-presa
- **brusselator_reaction_diffusion(A, B, k₁..k₄, D_x, D_y, spatial_position)** — Reacción-difusión
- **turing_instability_condition(D_ratio, J₁₁, J₂₂)** — 1 si D_y/D_x ≥ |J₂₂|/J₁₁

## SDFs definidas
- **reaction_potential_energy_surface(reactant_energy, ts_barrier, product_energy, intensity_threshold)** — PES continuo por tramos + trayectoria de reacción
- **molecular_collision_theory_visualization(σ, relative_velocity, intensity_threshold)** — Colisiones moleculares con disco de sección eficaz
- **transition_state_complex_visualization(ts_height, reactant_radius, product_radius, ts_radius, intensity_threshold)** — Complejo activado reactivos→TS→productos
- **michaelis_menten_kinetics_visualization(substrate_scale, vmax, km, intensity_threshold)** — Curva de saturación + complejo ES
- **chemical_oscillations_bz_pattern(time, wave_number, intensity_threshold)** — BZ espiral (atan)
- **reaction_diffusion_system(D, k, time, intensity_threshold)** — Sistema R-D
- **heterogeneous_catalysis_surface(surface_sites, adsorption_rate, reaction_rate, intensity_threshold)** — Catálisis superficial con sitios anulares
- **polymerization_kinetics_visualization(monomer, k_init, k_prop, intensity_threshold)** — 5 cadenas en crecimiento + monómeros
- **molecular_self_assembly_micelles(surfactant, CMC, intensity_threshold)** — Anillo de micelas (frecuencia ∝ exceso sobre CMC)
- **chemical_clock_temporal_oscillations(time, period, amplitude, intensity_threshold)** — Reloj químico pulsante + indicador de fase
- **concentration_gradient_reactor(inlet_concentration, k, D, intensity_threshold)** — Reactor de flujo

## Notas
- RRKM y Marcus son formulaciones conceptuales simplificadas.
- Los mecanismos complejos devuelven arreglos de concentraciones [A, B, C] para cada instante.
- La cinética enzimática incluye inhibición (competitiva, no competitiva, acompetitiva) y mecanismos de múltiples sustratos.
- Las SDFs de autoensamblaje y polimerización son representaciones geométricas del proceso.
- La condición de Turing permite explorar formación de patrones por difusión.
- Corregido 2026-10-17: carácter `>` inválido (L155, Turing) → `step`; `alpha` es keyword del engine → `alpha_lv` (Lotka-Volterra); `atan2`→`atan(z,x)` (BZ); strings/`if` en inhibición M-M y multi-sustrato → selectores numéricos 0/1/2/3; arrays/`for`/`max_array` → 2 reactantes (TST), 2 interferentes (ISE-style), 2 puntos Hill, sistema 2×2 Onsager; `circle`/`plane([vec])` no son primitivas → discos/planos vía `custom`; `rrmk`→`rrkm` (typo); el def de M-M llamaba `michaelis_menten_kinetics` (no definida aquí) → inlinada; PES con tramos discontinuos → parábola interpolante continua por (0.3, e_A), (0.45, ts_barrier), (0.6, e_B); `position` keyword → `spatial_position` (Brusselator); 11 defs parametrizados con `segment_distance` de std_bridge_atomic.
- Verificado 2026-10-17: compila (exit 0); probe 22 nodos / 38 customs exactos; inlinado numérico de `segment_distance` OK; checks: PES continuo (0.27/0.045 en los empalmes, máx 0.298 en rc=0.375), k_colisión(1e11,1,50kJ,300K)=196.97, Hill n=1.2041, Turing 0.5<0.8→0 y 0.5≥0.4→1, reversible A(t=0)=1, M-M competitiva con I=K_i → 0.3333, autocatalítica B(0)=B₀.
- Re-verificado 2026-11-04 (post-fix del resolver, `_` como carácter de palabra): las expresiones emitidas no cambian porque esta librería no tenía colisiones param/prefijo; **17/17 evaluaciones escalares OK** (14 funciones; multi-sustrato con 3 mecanismos y Turing con 2 casos), incluidas k_colisión(1e11,1,50kJ,300K)=196.968, TST, Hill, M-M competitiva 0.333333 y las dos compuertas de Turing.
