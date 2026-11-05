# std_bridge_biochem.hm

**Capa:** Intermediario: Química-Biología (5.1/5.2 — Bioquímica y Bioquímica Estructural)
**Versión:** 1.0
**Fuente:** Lehninger "Biochemistry", Stryer "Biochemistry", PDB (Protein Data Bank)
**Dependencias:** std_math_core.hm, std_bridge_atomic.hm
**Fecha:** 6 de junio de 2026

## Cómo
Proporciona constantes bioquímicas escaladas (Avogadro, gas, Faraday, Boltzmann), funciones para geometría de estructura proteica (ángulos peptídicos, torsiones φ/ψ, hélices α, láminas β), energía de interacciones moleculares (puentes H, hidrofóbicas, iónicas, van der Waals), cinética enzimática (Michaelis-Menten, inhibiciones, Hill), termodinámica bioquímica (Gibbs, equilibrio, desnaturalización), potenciales de membrana (Nernst, Goldman-Hodgkin-Katz), y SDFs para visualizar hélices, láminas, proteínas globulares, bicapas, sitios activos, canales iónicos, ADN, y sistemas como el complejo enzima-sustrato, cadena de transporte electrónico, ciclo de Krebs y transducción de señal.

## Por qué
Modelar matemática y geométricamente la estructura, función e interacciones de biomoléculas para permitir visualización y simulación de sistemas bioquímicos en el Lenguaje Hermético.

## Para qué
Usada como puente entre la química y la biología para representar proteínas, membranas, ácidos nucleicos, cinéticas enzimáticas y metabolismo en un entorno de modelado 3D.

## Funciones principales
- `peptide_bond_angle()` — Ángulo del enlace peptídico (ω ≈ 180°)
- `ramachandran_phi_psi_allowed(phi, psi)` — Regiones permitidas del mapa de Ramachandran
- `hydrogen_bond_energy(distance, angle)` — Energía de puente de H (~5 kcal/mol)
- `hydrophobic_interaction_energy(buried_area)` — ΔG ≈ -1.3 kcal/mol·Å²
- `ionic_interaction_energy(q1, q2, distance)` — Interacción iónica en agua
- `van_der_waals_energy(epsilon, sigma, distance)` — Lennard-Jones
- `michaelis_menten_velocity(Vmax, Km, substrate_concentration)` — Velocidad enzimática
- `competitive_inhibition_velocity(Vmax, Km, Ki, S, I)` — Inhibición competitiva
- `hill_equation_cooperative(Vmax, K_half, n, S)` — Cooperatividad alostérica
- `biochemical_gibbs_free_energy(deltaG0, Q)` — ΔG = ΔG⁰ + RT·ln(Q)
- `nernst_membrane_potential(z, out, in)` — Potencial de Nernst
- `goldman_hodgkin_katz_potential(p_na, na_out, na_in, p_k, k_out, k_in, p_cl, cl_out, cl_in)` — GHK en forma estándar Na⁺/K⁺/Cl⁻ (~-68.7 mV típico)
- `protein_denaturation_free_energy(deltaH, deltaS, T)` — Desnaturalización
- `membrane_bending_energy(kappa, H, H0, kappa_G, K)` — Energía de Helfrich

## SDFs definidas
- `alpha_helix_structure(radius, pitch, residues, residue_spacing)` — Hélice α
- `beta_sheet_structure(strand_length, strand_width, strand_spacing, antiparallel)` — Lámina β
- `globular_protein(radius, surface_roughness, core_density)` — Proteína globular
- `lipid_bilayer(thickness, fluidity, curvature)` — Bicapa lipídica
- `enzyme_active_site(pocket_radius, entrance_width, specificity)` — Sitio activo
- `ion_channel(pore_radius, selectivity_filter_length, gating_state)` — Canal iónico
- `dna_double_helix(helix_radius, pitch, base_pair_spacing)` — ADN doble hélice
- `enzyme_substrate_complex(enzyme_radius, substrate_radius, binding_affinity)` — Complejo ES
- `electron_transport_chain(complex_spacing, membrane_thickness)` — Cadena de transporte
- `krebs_cycle_metabolism(cycle_radius, metabolite_count)` — Ciclo de Krebs
- `signal_transduction_complex(receptor_size, ligand_size, amplification_factor)` — Transducción

## Notas
Las constantes están escaladas para visualización. Las interacciones moleculares son aproximaciones energéticas. Las cinéticas enzimáticas siguen modelos clásicos (Michaelis-Menten, inhibiciones, Hill). Los sistemas bioquímicos son abstracciones geométricas de procesos reales.

Corregido 2026-10-17: `alpha` es palabra reservada (renombrada `alpha_factor`); los `def` no aceptan `let` (expresiones inline); `union` es binario; el lenguaje no tiene `for`/listas de longitud dinámica: GHK pasó a la forma estándar de 3 iones, la cadena de transporte despliega sus 4 complejos (escalados por `complex_spacing`) y el ciclo de Krebs reemplazó el bucle de metabolitos por nódulos periódicos (cos(n·θ)). El par de bases del ADN ahora es un segmento horizontal real entre hebras (vía `segment_distance`).
