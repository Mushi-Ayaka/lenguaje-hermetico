# std_bridge_biophysics.hm

**Capa:** Intermediario Química-Biología (5.5 — Biofísica Molecular)
**Versión:** 1.0
**Fuente:** "Biophysics" by Glaser
**Dependencias:** std_math_core.hm, std_math_algebra.hm, std_phys_statmech.hm, std_bridge_biochem.hm, std_bridge_quantchem.hm
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
- `pi_pi_stacking_energy(distance, offset, type)` — Apilamiento π-π
- `rmsd_calculation(coords1, coords2)` — RMSD entre conformaciones
- `radius_of_gyration(coords, center_of_mass)` — Radio de giro
- `b_factor_fluctuation(mean_square_displacement)` — Factor B cristalográfico
- `brownian_motion_displacement(D, time)` — Desplazamiento browniano
- `dissociation_constant(kon, koff)` — Kd
- `binding_free_energy(kd, T)` — ΔG de unión
- `itc_binding_thermodynamics(deltaH, deltaS, deltaCp, T)` — Termodinámica ITC
- `channel_conductance(current, voltage)` — Conductancia de canal
- `nernst_potential(in, out, charge, T)` — Potencial de Nernst

## SDFs definidas
- `protein_folding_visualization(unfolded, folded, progress, threshold)` — Plegamiento animado
- `hydrogen_bond_in_protein(donor, acceptor, angle, threshold)` — Puente de H visual
- `conformational_dynamics_visualization(ground, excited, weights, threshold)` — Paisaje energético
- `protein_ligand_binding_visualization(site, ligand, energy, threshold)` — Unión molecular
- `ion_channel_visualization(pore_radius, length, filter, threshold)` — Canal iónico 3D
- `biological_membrane_visualization(count, thickness, threshold)` — Bicapa lipídica

## Notas
La biofísica molecular aplica principios físicos a sistemas biológicos. El plegamiento está gobernado por fuerzas termodinámicas. Las interacciones no covalentes estabilizan estructuras. La unión proteína-ligando es clave para función y diseño de fármacos.
