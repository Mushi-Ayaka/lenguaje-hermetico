# std_bridge_biophysics.hm

**Capa:** Intermediario Química-Biología (5.5 — Biofísica Molecular)
**Versión:** 1.0
**Fuente:** "Biophysics" by Glaser
**Dependencias:** std_math_core.hm, std_bridge_atomic.hm
**Fecha:** 6 de junio de 2026

## Cómo
Define constantes fundamentales (Boltzmann, Avogadro, gas, Planck) y funciones para plegamiento de proteínas (energía libre, Tm, estabilidad térmica/cinética), fuerzas intermoleculares (hidrofóbica, puente de H, electrostática Debye-Hückel, ión-π, π-π stacking), dinámica conformacional (RMSD, radio de giro, B-factor, difusión rotacional, movimiento browniano), unión proteína-ligando (Kd, energía de unión, termodinámica ITC, especificidad), y canales iónicos (conductancia, probabilidad de apertura, permeabilidad, Nernst, transporte).

## Por qué
Aplicar principios físicos (termodinámica, electrostática, mecánica estadística) a sistemas biológicos para modelar plegamiento, interacciones no covalentes, y dinámica molecular.

## Para qué
Usada para describir cuantitativamente fenómenos biofísicos como estabilidad de proteínas, reconocimiento molecular, dinámica conformacional, y transporte a través de membranas.

## Funciones principales
- `folding_free_energy(hydrophobic, hbonds, vdw, entropy)` — ΔG de plegamiento
- `protein_melting_temperature(deltaH, deltaS)` — Temperatura de fusión
- `thermal_stability(deltaG, T, Tm)` — Fracción plegada a temperatura dada
- `kinetic_stability(unfolding_rate, refolding_rate)` — Razón de estabilidad cinética
- `hydrophobic_interaction_energy(surface_area, T)` — Energía hidrofóbica en J/mol
- `hydrogen_bond_energy(distance, angle, dielectric)` — Energía de puente de H
- `electrostatic_interaction(q1, q2, r, I, epsilon)` — Debye-Hückel
- `pi_pi_stacking_energy(distance, offset, type)` — Apilamiento π-π (type numérico: 0=face, 1=T-shaped)
- `rmsd_calculation(x1, y1, z1, x2, y2, z2)` — RMSD de una correspondencia de puntos
- `radius_of_gyration(sum_squared_distance, point_count)` — Radio de giro (suma Σ|rᵢ−COM|² acumulada por la aplicación)
- `binding_specificity(target, off_target_mean, N)` — Especificidad (media off-target/target)^N
- `b_factor_fluctuation(mean_square_displacement)` — Factor B cristalográfico
- `brownian_motion_displacement(D, time)` — Desplazamiento browniano
- `dissociation_constant(kon, koff)` — Kd
- `binding_free_energy(kd, T)` — ΔG de unión
- `itc_binding_thermodynamics(deltaH, deltaS, deltaCp, T)` — Termodinámica ITC
- `channel_conductance(current, voltage)` — Conductancia de canal
- `nernst_potential(in, out, charge, T)` — Potencial de Nernst

## SDFs definidas
- `protein_folding_visualization(unfolded_radius, folding_progress, threshold)` — Plegamiento paramétrico (compactación)
- `hydrogen_bond_in_protein(dx, dy, dz, ax, ay, az, angle, threshold)` — Puente de H visual (cápsula + halo)
- `conformational_dynamics_visualization(gx..ez, ground_weight, excited_weight, threshold)` — Estado fundamental/excitado + transición
- `protein_ligand_binding_visualization(lx, ly, lz, ligand_radius, binding_energy, threshold)` — Unión molecular
- `ion_channel_visualization(pore_radius, length, filter, threshold)` — Canal iónico 3D (pared vía `subtract`)
- `biological_membrane_visualization(lipid_count, thickness, threshold)` — Bicapa lipídica (lipid_count = frecuencia espacial)

## Notas
La biofísica molecular aplica principios físicos a sistemas biológicos. El plegamiento está gobernado por fuerzas termodinámicas. Las interacciones no covalentes estabilizan estructuras. La unión proteína-ligando es clave para función y diseño de fármacos.

Corregido 2026-10-17: el lenguaje no soporta listas dinámicas, `for`, `append`, `if`/`<` ni primitivas posicionadas. Los `def` SDF se reescribieron como modelos paramétricos (coordenadas escalares y frecuencias en vez de listas), `difference(...)` → `subtract(a, b)` nativo, y `pi_pi_stacking_energy` usa selector numérico con `step`.
