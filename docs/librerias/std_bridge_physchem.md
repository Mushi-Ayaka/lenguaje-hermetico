# std_bridge_physchem.hm

**Capa:** Intermediario Física-Química (3.4 - Fisicoquímica)
**Versión:** 1.0
**Fuente:** "Physical Chemistry" by Atkins & de Paula
**Dependencias:** std_math_core.hm, std_bridge_atomic.hm
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
- **entropy_of_mixing(x₁, x₂)** — ΔS_mix = -R Σ x_i ln x_i (2 componentes)
- **gibbs_phase_rule(components, phases)** — F = C - P + 2
- **clausius_clapeyron_equation(P₁, T₁, T₂, ΔH_vap)** — Presión de vapor vs T
- **raoults_law_vapor_pressure(x_i, P_i°)** — Presión de vapor ideal
- **osmotic_pressure_vanthoff(c, T)** / **osmotic_pressure_virial(c, T, B, C)** — Presión osmótica
- **arrhenius_equation(A, E_a, T)** — k = A·exp(-E_a/RT)
- **eyring_equation(T, ΔG‡)** — Teoría del estado de transición
- **half_life_nth_order([A]₀, k, n)** — t½ (n=1: ln2/k; n≠1: forma general, sin `if`)
- **initial_rate_method(k, [A], n_A, [B], n_B)** — v = k·[A]^nA·[B]^nB (2 reactantes)
- **michaelis_menten_kinetics([S], V_max, K_M)** — Cinética enzimática
- **beer_lambert_law(ε, c, l)** — A = ε·c·l
- **competitive_inhibition / noncompetitive_inhibition / uncompetitive_inhibition** — Inhibición enzimática

## SDFs definidas
- **gibbs_free_energy_surface(reactant_energy, ts_energy, product_energy, intensity_threshold)** — Perfil de energía libre 3D continuo (x=reacción, y=temperatura)
- **chemical_equilibrium_dynamics(forward_rate, reverse_rate, intensity_threshold)** — Equilibrio dinámico
- **arrhenius_reaction_profile(temperature, activation_energy, intensity_threshold)** — Barrera de Arrhenius (k normalizada A=1)
- **enzyme_catalysis_michaelis_menten(substrate_scale, vmax, km, intensity_threshold)** — Curva de saturación
- **liquid_vapor_phase_transition(temperature, critical_temperature, intensity_threshold)** — Diagrama de fases P-V
- **molecular_diffusion_fick(diffusion_coefficient, time, intensity_threshold)** — 2ª ley de Fick
- **chemical_oscillations_bz(time, wave_number, intensity_threshold)** — Belousov-Zhabotinsky
- **reverse_osmosis_membrane(pressure, solute_concentration, intensity_threshold)** — Ósmosis inversa
- **crystallization_from_solution(supersaturation, nucleation_rate, intensity_threshold)** — Cristalización
- **langmuir_adsorption_isotherm(pressure, adsorption_sites, binding_energy, intensity_threshold)** — Isoterma de Langmuir
- **continuous_stirred_tank_reactor(inflow_rate, reaction_rate, intensity_threshold)** — CSTR

## Notas
- Constantes escaladas para visualización.
- Cinética y termodinámica usan unidades coherentes internamente.
- La inhibición enzimática cubre los tres tipos clásicos: competitiva, no competitiva y acompetitiva.
- Las SDFs de fenómenos avanzados (BZ, ósmosis inversa, cristalización) son representaciones conceptuales.
- Corregido 2026-10-17: carácter `<` inválido eliminado (perfil de energía → `step`); `if/else` en `half_life_nth_order` → selectores `step`; arrays/`for` en `entropy_of_mixing` (→ 2 fracciones molares), `osmotic_pressure_virial` (→ B, C), `initial_rate_method` (→ 2 reactantes) y en defs (cristalización → 5 núcleos desplegados; Langmuir → anillo de moléculas con nódulos por sitio); `union_array`/`append`/`let [S]` eliminados; primitivas vectoriales (`sphere([vec],r)`, `plane([vec],d)`) → `custom`; `atan2` → `atan(z,x)` (BZ); `step(a,b)` con argumentos invertidos corregidos (frentes de reacción; membrana de ósmosis: ahora hay flujo solo cuando P > Π); escalas rotas: perfil Arrhenius usaba k = 1e13·exp(...) → intensidad ~1968 (superficie plana) → k normalizada (A=1.0, rango 0..1); perfil de Gibbs con saltos (0.45→0) → parábola interpolante continua por (0.3; 0.09·E_r), (0.45; E_ts), (0.6; 0.01·E_p); defs sin `let` (todo inlinado; `w` = tiempo del shader).
- Verificado 2026-10-17: compila (exit 0); probe 22 nodos / 19 customs exactos; checks: Gibbs continuo (0.45/0.45 y 0.03/0.03 en los empalmes), ΔS_mix(½,½)=5.763 J/(mol·K), virial(0.1,300)=264.4, t½(1)=1.3863 y t½(2,10)=0.2, v_init=18, k_arrh=0.9802, θ_Langmuir=0.8737 (E_b=−0.05 eV, P=1), difusión C(0)=0.892.
