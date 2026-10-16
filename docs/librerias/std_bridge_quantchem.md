# std_bridge_quantchem.hm

**Capa:** Intermediario Física-Química (3.3 - Química Cuántica)
**Versión:** 1.0
**Fuente:** "Modern Quantum Chemistry" by Szabo & Ostlund
**Dependencias:** std_math_core.hm, std_math_algebra.hm, std_math_calculus.hm, std_phys_qed.hm, std_phys_statmech.hm
**Fecha:** 6 de junio de 2026

## Cómo
Implementa el formalismo de la química cuántica computacional: hamiltoniano molecular (no relativista), aproximación de Born-Oppenheimer, teoría LCAO (combinación lineal de orbitales atómicos), ecuaciones de Roothaan-Hall (Hartree-Fock), teoría del funcional de la densidad DFT (Kohn-Sham, LDA), métodos post-HF (MP2, CISD, CCSD), descriptores de reactividad (Fukui, dureza/química, índice de electrofilia) y teoría del estado de transición cuántica. Incluye SDFs para orbitales moleculares, densidad electrónica, superficie de energía potencial, enlace químico, correlación electrónica, estados excitados, transferencia de carga, vibraciones moleculares, efecto Jahn-Teller y polarización por solvente.

## Por qué
La química cuántica reduce la química a física aplicada. Esta librería traduce los operadores y métodos de la estructura electrónica a funciones computables y formas visualizables dentro del Lenguaje Hermético.

## Para qué
Modelar estructura electrónica molecular, orbitales moleculares (HOMO/LUMO), superficies de energía potencial, densidad de enlace, correlación electrónica, estados excitados, transferencia de carga, modos vibracionales y efectos de solvente.

## Funciones principales
- **molecular_hamiltonian(electron_positions, nuclei_positions, nuclei_charges)** — Ĥ = T̂ + V̂ₑₙ + V̂ₑₑ + V̂ₙₙ
- **born_oppenheimer_energy(electronic_energy, nuclear_repulsion)** — Separación de movimientos
- **molecular_orbital_lcao(position, atomic_orbitals, coefficients)** — ψ = Σ c_μi·φ_μ
- **roothaan_hall_equations(F, S, ε)** — FC = SCε (autovalores aproximados)
- **dft_total_energy(ρ, T_s, V_ext, J, E_xc)** — E[ρ] = T_s + V_ext + J + E_xc
- **lda_exchange_correlation_energy(ρ, r)** — ε_xc(ρ) ∝ -ρ^{1/3}
- **fukui_function(lumo_density)** — f(r) ≈ |ψ_LUMO(r)|²
- **chemical_hardness(IP, EA)** / **chemical_softness(η)** — η = (IP-EA)/2
- **electrophilicity_index(μ, η)** — ω = μ²/(2η)
- **mp2_correlation_energy(⟨ij||ab⟩, ε_occ, ε_virt)** — E_corr MP2
- **coupled_cluster_energy(E₀, t₁, t₂)** — E_CCSD
- **kohn_sham_band_gap(ε_HOMO, ε_LUMO)** — Δ_KS = ε_LUMO - ε_HOMO

## SDFs definidas
- **molecular_orbitals_visualization(atomic_positions, orbital_coefficients, intensity_threshold)** — Orbitales como superficies de densidad
- **molecular_electron_density(nuclei_positions, nuclei_charges, intensity_threshold)** — Densidad electrónica total
- **potential_energy_surface_visualization(reaction_coordinate, energy_profile, intensity_threshold)** — Superficie de energía potencial 2D
- **frontier_orbitals_visualization(homo_density, lumo_density, intensity_threshold)** — HOMO/LUMO
- **chemical_bond_density(atom1_position, atom2_position, bond_order, intensity_threshold)** — Densidad de enlace
- **electron_correlation_hole(reference_density, correlated_density, intensity_threshold)** — Agujero de correlación
- **excited_states_absorption(ground_state, excited_states, transition_dipoles, intensity_threshold)** — Absorción óptica
- **charge_transfer_states(donor_density, acceptor_density, coupling, intensity_threshold)** — Transferencia de carga
- **molecular_vibrations_normal_modes(equilibrium_geometry, vibrational_frequencies, intensity_threshold)** — Modos normales
- **jahn_teller_distortion(symmetric_geometry, distortion_strength, intensity_threshold)** — Distorsión Jahn-Teller
- **solvent_polarization_effect(solute_density, dielectric_constant, intensity_threshold)** — Polarización por solvente

## Notas
- Los cálculos son aproximaciones conceptuales, no sustitutos de paquetes cuánticos reales.
- Las integrales de dos electrones y los elementos de matriz están simplificados.
- Los métodos post-HF (MP2, CISD, CCSD) son representaciones del flujo algorítmico.
- DFT usa funcional LDA; no se implementan GGA ni híbridos.
- Ideal para visualizar conceptos de estructura electrónica en 3D.
