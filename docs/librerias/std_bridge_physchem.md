# std_bridge_physchem.hm

**Capa:** Intermediario Física-Química (3.4 - Fisicoquímica)
**Versión:** 1.0
**Fuente:** "Physical Chemistry" by Atkins & de Paula
**Dependencias:** std_math_core.hm, std_math_algebra.hm, std_math_calculus.hm, std_phys_statmech.hm, std_phys_thermo.hm, std_bridge_atomic.hm
**Fecha:** 6 de junio de 2026

## Cómo
Define constantes fisicoquímicas (R, F, k_B, N_A) y funciones de termodinámica química (ΔG, K_eq, Van't Hoff, entropía de mezcla), equilibrio de fases (Gibbs, Clapeyron, Clausius-Clapeyron, Raoult, Henry), propiedades coligativas (descenso crioscópico, elevación ebulloscópica, ósmosis), cinética química (Arrhenius, Eyring, órdenes de reacción, Michaelis-Menten) y fotoquímica (Beer-Lambert, rendimiento cuántico). Incluye SDFs para visualizar superficies de energía, equilibrio dinámico, perfiles Arrhenius, catálisis enzimática, transiciones de fase, difusión de Fick y fenómenos avanzados (oscilaciones BZ, ósmosis inversa, cristalización, Langmuir, CSTR).

## Por qué
La fisicoquímica es el puente disciplinar entre la física y la química. Esta librería expresa en funciones y formas los principios que gobiernan la espontaneidad, el equilibrio, la velocidad de reacción y las propiedades de las soluciones.

## Para qué
Modelar equilibrio químico, cinética de reacciones, catálisis enzimática, transiciones de fase, diagramas de fase, difusión molecular, oscilaciones químicas, adsorción y reactores de flujo continuo.

## Funciones principales
- **gibbs_free_energy_reaction(enthalpy_change, entropy_change, temperature)** — ΔGº = ΔHº - TΔSº
- **equilibrium_constant(gibbs_free_energy, temperature)** — K_eq = exp(-ΔGº/RT)
- **vant_hoff_equation(ΔH, T₁, T₂)** — Dependencia de K con temperatura
- **entropy_of_mixing(mole_fractions)** — ΔS_mix = -R Σ x_i ln x_i
- **gibbs_phase_rule(components, phases)** — F = C - P + 2
- **clausius_clapeyron_equation(P₁, T₁, T₂, ΔH_vap)** — Presión de vapor vs T
- **raoults_law_vapor_pressure(x_i, P_i°)** — Presión de vapor ideal
- **osmotic_pressure_vanthoff(c, T)** / **osmotic_pressure_virial(c, T, B)** — Presión osmótica
- **arrhenius_equation(A, E_a, T)** — k = A·exp(-E_a/RT)
- **eyring_equation(T, ΔG‡)** — Teoría del estado de transición
- **michaelis_menten_kinetics([S], V_max, K_M)** — Cinética enzimática
- **beer_lambert_law(ε, c, l)** — A = ε·c·l
- **competitive_inhibition / noncompetitive_inhibition / uncompetitive_inhibition** — Inhibición enzimática

## SDFs definidas
- **gibbs_free_energy_surface(reaction_coordinate, temperature_range, intensity_threshold)** — Perfil de energía libre 3D
- **chemical_equilibrium_dynamics(forward_rate, reverse_rate, intensity_threshold)** — Equilibrio dinámico
- **arrhenius_reaction_profile(temperature, activation_energy, intensity_threshold)** — Barrera de Arrhenius
- **enzyme_catalysis_michaelis_menten(substrate_range, vmax, km, intensity_threshold)** — Curva de saturación
- **liquid_vapor_phase_transition(temperature, critical_temperature, intensity_threshold)** — Diagrama de fases P-V
- **molecular_diffusion_fick(diffusion_coefficient, time, intensity_threshold)** — 2ª ley de Fick
- **chemical_oscillations_bz(time, rate_constants, intensity_threshold)** — Belousov-Zhabotinsky
- **reverse_osmosis_membrane(pressure, solute_concentration, intensity_threshold)** — Ósmosis inversa
- **crystallization_from_solution(supersaturation, nucleation_rate, intensity_threshold)** — Cristalización
- **langmuir_adsorption_isotherm(pressure, adsorption_sites, binding_energy, intensity_threshold)** — Isoterma de Langmuir
- **continuous_stirred_tank_reactor(inflow_rate, reaction_rate, intensity_threshold)** — CSTR

## Notas
- Constantes escaladas para visualización.
- Cinética y termodinámica usan unidades coherentes internamente.
- La inhibición enzimática cubre los tres tipos clásicos: competitiva, no competitiva y acompetitiva.
- Las SDFs de fenómenos avanzados (BZ, ósmosis inversa, cristalización) son representaciones conceptuales.
