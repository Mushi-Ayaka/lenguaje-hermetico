# std_bridge_statmech_mol.hm

**Capa:** Intermediario Física-Química (3.6 - Mecánica Estadística Molecular)
**Versión:** 1.0
**Fuente:** "Molecular Statistical Mechanics" by McQuarrie
**Dependencias:** std_math_core.hm, std_bridge_atomic.hm
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
- **rotational_partition_function_nonlinear(T, Θ_A, Θ_B, Θ_C, σ)** — q_rot = √(π/σ)·(T³/Θ_AΘ_BΘ_C)^{½} (3 temperaturas escalares)
- **vibrational_partition_function_harmonic(Θ_vib, T)** — q_vib = 1/(1 - exp(-Θ_vib/T))
- **total_vibrational_partition_function(Θ₁, Θ₂, Θ₃, T)** — Producto de 3 modos (antes array)
- **electronic_partition_function(ε₁, g₁, ε₂, g₂, T)** — q_elec = Σ g_i·exp(-ε_i/kT) (2 niveles, ε en eV)
- **sackur_tetrode_entropy(q_trans, N, T)** — S_trans = Nk·[ln(q_trans·e/N) + 5/2]
- **molecular_heat_capacity(C_V_trans, C_V_rot, C_V_vib, C_V_elec)** — Capacidad calorífica total
- **maxwell_boltzmann_molecular(v, m, T)** — f(v) = 4π·(m/2πkT)^{3/2}·v²·exp(-mv²/2kT)
- **most_probable_speed(m, T)** / **average_speed(m, T)** / **root_mean_square_speed(m, T)** — Velocidades características
- **mean_free_path(n, σ)** — λ = 1/(√2·n·σ)
- **collision_frequency(n, σ, v_avg)** — Z = √2·n·σ·v_avg
- **kinetic_pressure(n, m, v_rms)** — P = ⅓·n·m·v_rms²
- **gas_diffusion_coefficient(λ, v_avg)** — D = ⅓·λ·v_avg
- **gas_viscosity(n, m, λ, v_avg)** — η = ⅓·n·m·λ·v_avg
- **gas_thermal_conductivity(n, λ, v_avg, C_V)** — κ = ⅓·n·λ·v_avg·C_V
- **grahams_law_effusion(m₁, m₂)** — v₁/v₂ = √(m₂/m₁)
- **lennard_jones_potential(r, ε, σ)** — V(r) = 4ε[(σ/r)^{12} - (σ/r)^{6}]
- **second_virial_coefficient(T, σ, ε)** — B(T) = 2π∫[1 - exp(-V_LJ/kT)]·r² dr (numérico, LJ inline; antes recibía un callback de potencial)
- **radial_distribution_function(r, T, ρ, ε, σ)** — g(r) = exp(-V_LJ(r)/kT) (LJ inline; antes callback)
- **virial_equation_of_state(ρ, T, B, C)** — P/(ρkT) = 1 + Bρ + Cρ²

## SDFs definidas
- **molecular_velocity_distribution_3d(T, m, intensity_threshold)** — Distribución de Maxwell-Boltzmann 3D (gaussiana normalizada como altura + esfera guía)
- **molecular_partition_functions_visualization(m, V, Θ_rot, Θ_vib, intensity_threshold)** — Log-ratios q_trans, q_rot, q_vib normalizados a T=100
- **lennard_jones_potential_visualization(ε, σ, T, intensity_threshold)** — Potencial LJ + g(r)
- **molecular_collisions_gas(n, intensity_threshold)** — Trayectorias con movimiento propio + λ visual (antes tenía params muertos T, m)
- **molecular_vibrational_modes(f₁, f₂, f₃, T, intensity_threshold)** — Modos normales vibratorios (amplitud ∝ √(T/300)/f)
- **rotational_transitions_spectrum(B, T, intensity_threshold)** — 10 líneas rotacionales con población (2J+1)·exp(-BJ(J+1)/kT), k_B≈0.695 cm⁻¹/K
- **gas_liquid_phase_transition(T, T_c, intensity_threshold)** — Nivel de líquido ∝ (1-T/T_c) + burbuja en ventana cerca de T_c
- **molecular_clustering_aggregation(ε, T, intensity_threshold)** — 5 moléculas con persistencia exp(-i·T·0.01/ε)
- **diffusion_in_porous_medium(D, φ, τ, intensity_threshold)** — Perfil gaussiano con D_eff = D·φ/τ² (ancho ∝ w)
- **surface_adsorption_isotherm(P, N, E, intensity_threshold)** — Isoterma de Langmuir: N sitios con compuerta 1-step(θ, i/N), K con k_B en eV
- **molecular_conformational_transitions(E_barrier, T, intensity_threshold)** — Dos confórmeros + puente + barrera parabólica

## Notas
- Las funciones de partición asumen moléculas ideales independientes (gas ideal).
- La capacidad calorífica vibracional depende de la temperatura; a baja T solo contribuyen traslación y rotación.
- El coeficiente del virial B(T) usa una cuadratura numérica sobre el potencial de Lennard-Jones inline.
- Las SDFs de colisiones y clustering son dinámicas (usan w para tiempo).
- Las transiciones conformacionales modelan equilibrio cis-trans con barrera de energía.
- **Corregido 2026-10-17:**
  - `volume` es keyword del lexer → renombrado a `vol` en `translational_partition_function` y `molecular_partition_functions_visualization`.
  - Eliminados `for`/arrays/callbacks de potencial/tipos de primitivas inválidas (`sphere([vec],r)`, `cylinder([vec],[vec],r)`, `plane([vec],d)`); funciones escalarizadas (rotacional no lineal con Θ_A, Θ_B, Θ_C; vibracional con 3 modos; electrónica 2 niveles; `second_virial_coefficient(T, σ, ε)` y `radial_distribution_function(r, T, ρ, ε, σ)` con LJ inline; virial con 2 coeficientes).
  - `molecular_collisions_gas` perdía `temperature` y `mass` sin uso → firma `(number_density, intensity_threshold)`.
  - `surface_adsorption_isotherm` usaba k_B en J con energía en eV (K≈0, cobertura nula) → k_B en eV; 10 sitios con compuertas `1-step(θ, i/N)`.
  - `export`: isoterma con 10 `step` anidados reescrita como cadena `min` plana (bug de balance de paréntesis en el patrón árbol).
  - **Bug del resolver (herm_resolver.cpp `substExprText`):** `_` no contaba como carácter de palabra → el param `temperature` se sustituía dentro de `rotational_temperature`/`vibrational_temperature`, emitiendo `rotational_(...)` no inlineable. Fix: `isWord = isalnum || '_'` + rebuild. Afecta a cualquier lib con params homónimos-sufijo.
- **Verificado 2026-10-17:** compila (exit 0); probe de 11 instancias → 17 custom (esperado); sin `for`/`atan2`/`<`/`>` en exprs; checks: vel sdf(0)=−0.85, esfera r=0.164235, part sdf(x=0)=−0.1, LJ V(r_min)=−1.0 (sdf −0.02 / −0.12 / −0.08699), λ_vis=0.212132, vibmodes(0,0,0)=−0.08, espectro alturas (2J+1)·exp(−J(J+1)B/kT)·0.2 (pico J=3, h=0.7606), phase sdf(0)=−0.24 y burbuja r(T=T_c)=0.05, cluster r₀=0.08 / r₄=0.0300003, diff sdf(0)=−0.13, adsorción cov=0.01034 (1 sitio activo, sitio0 −0.02), conformacional blob −0.1 / esfera −0.05. Regresión DL-14 (test_dl14_fn_lets) OK tras el fix del resolver.
