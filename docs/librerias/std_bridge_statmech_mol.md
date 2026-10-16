# std_bridge_statmech_mol.hm

**Capa:** Intermediario Física-Química (3.6 - Mecánica Estadística Molecular)
**Versión:** 1.0
**Fuente:** "Molecular Statistical Mechanics" by McQuarrie
**Dependencias:** std_math_core.hm, std_math_algebra.hm, std_math_calculus.hm, std_phys_statmech.hm, std_bridge_atomic.hm, std_bridge_quantchem.hm
**Fecha:** 6 de junio de 2026

## Cómo
Define funciones de partición moleculares (translacional, rotacional para lineal y no lineal, vibracional armónico, electrónica) y derivadas termodinámicas (energía interna, entropías de Sackur-Tetrode, rotacional, vibracional, capacidades caloríficas). Incluye teoría cinética de gases (Maxwell-Boltzmann, velocidades características, camino libre medio, frecuencia de colisiones), propiedades de transporte (difusión, viscosidad, conductividad térmica, ley de Graham), sistemas de interacción (Lennard-Jones, coeficiente del virial B(T), función de distribución radial g(r), ecuación de estado virial). Provee SDFs para distribuciones de velocidad, funciones de partición, potencial LJ con g(r), colisiones moleculares, transiciones rotacionales, transición de fase gas-líquido, agregación molecular, difusión en medio poroso, adsorción superficial y transiciones conformacionales.

## Por qué
Conecta el mundo microscópico de las moléculas individuales con las propiedades macroscópicas de la materia. Las funciones de partición contienen toda la información termodinámica de un sistema molecular.

## Para qué
Calcular funciones de partición moleculares y propiedades termodinámicas derivadas, modelar distribuciones de velocidad molecular (Maxwell-Boltzmann), propiedades de transporte en gases, potenciales intermoleculares, estructura de líquidos (g(r)), diagramas de fase, agregación molecular, adsorción y difusión en medios porosos.

## Funciones principales
- **total_molecular_partition_function(q_trans, q_rot, q_vib, q_elec)** — q = q_trans·q_rot·q_vib·q_elec
- **translational_partition_function(m, T, V)** — q_trans = (2πmkT/h²)^{3/2}·V
- **rotational_partition_function_linear(T, Θ_rot, σ)** — q_rot = T/(σ·Θ_rot)
- **rotational_partition_function_nonlinear(T, [Θ_A,Θ_B,Θ_C], σ)** — q_rot = √(π/σ)·(T³/Θ_AΘ_BΘ_C)^{½}
- **vibrational_partition_function_harmonic(Θ_vib, T)** — q_vib = 1/(1 - exp(-Θ_vib/T))
- **electronic_partition_function(ε_i, g_i, T)** — q_elec = Σ g_i·exp(-ε_i/kT)
- **sackur_tetrode_entropy(q_trans, N, T)** — S_trans = Nk·[ln(q_trans·e/N) + 5/2]
- **molecular_heat_capacity(C_V_trans, C_V_rot, C_V_vib, C_V_elec)** — Capacidad calorífica total
- **maxwell_boltzmann_molecular(v, m, T)** — f(v) = 4π·(m/2πkT)^{3/2}·v²·exp(-mv²/2kT)
- **most_probable_speed(m, T)** / **average_speed(m, T)** / **root_mean_square_speed(m, T)** — Velocidades características
- **mean_free_path(n, σ)** — λ = 1/(√2·n·σ)
- **collision_frequency(n, σ, v_avg)** — Z = √2·n·σ·v_avg
- **gas_diffusion_coefficient(λ, v_avg)** — D = ⅓·λ·v_avg
- **gas_viscosity(n, m, λ, v_avg)** — η = ⅓·n·m·λ·v_avg
- **gas_thermal_conductivity(n, λ, v_avg, C_V)** — κ = ⅓·n·λ·v_avg·C_V
- **grahams_law_effusion(m₁, m₂)** — v₁/v₂ = √(m₂/m₁)
- **lennard_jones_potential(r, ε, σ)** — V(r) = 4ε[(σ/r)^{12} - (σ/r)^{6}]
- **second_virial_coefficient(T, potential_function)** — B(T) = 2π∫[1 - exp(-V/kT)]·r² dr (simplificado)
- **radial_distribution_function(r, T, ρ, potential)** — g(r) = exp(-βV(r))·(1+ρ·0.1)
- **virial_equation_of_state(ρ, T, [B, C, ...])** — P/(ρkT) = 1 + Bρ + Cρ² + ...

## SDFs definidas
- **molecular_velocity_distribution_3d(T, m, intensity_threshold)** — Distribución de Maxwell-Boltzmann 3D
- **molecular_partition_functions_visualization(T, params, intensity_threshold)** — q_trans, q_rot, q_vib vs T
- **lennard_jones_potential_visualization(ε, σ, T, intensity_threshold)** — Potencial LJ + g(r)
- **molecular_collisions_gas(n, T, m, intensity_threshold)** — Trayectorias y camino libre medio
- **molecular_vibrational_modes(frequencies, T, intensity_threshold)** — Modos normales vibratorios
- **rotational_transitions_spectrum(B, T, intensity_threshold)** — Líneas rotacionales con población
- **gas_liquid_phase_transition(T, T_critical, intensity_threshold)** — Curva de coexistencia + punto crítico
- **molecular_clustering_aggregation(interaction_strength, T, intensity_threshold)** — Clusters y agregación
- **diffusion_in_porous_medium(D, porosity, tortuosity, intensity_threshold)** — Difusión efectiva en medio poroso
- **surface_adsorption_isotherm(P, sites, binding_energy, intensity_threshold)** — Isoterma de Langmuir
- **molecular_conformational_transitions(energy_barrier, T, intensity_threshold)** — Cis-trans y barrera conformacional

## Notas
- Las funciones de partición asumen moléculas ideales independientes (gas ideal).
- La capacidad calorífica vibracional depende de la temperatura; a baja T solo contribuyen traslación y rotación.
- El coeficiente del virial B(T) usa un potencial de esfera dura simplificado.
- Las SDFs de colisiones y clustering son dinámicas (usan w para tiempo).
- Las transiciones conformacionales modelan equilibrio cis-trans con barrera de energía.
