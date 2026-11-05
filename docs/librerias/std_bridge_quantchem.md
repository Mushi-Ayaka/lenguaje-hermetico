# std_bridge_quantchem.hm

**Capa:** Intermediario Física-Química (3.3 - Química Cuántica)
**Versión:** 1.0
**Fuente:** "Modern Quantum Chemistry" by Szabo & Ostlund
**Dependencias:** std_math_core.hm, std_bridge_atomic.hm
**Fecha:** 6 de junio de 2026

## Cómo
Implementa el formalismo de la química cuántica computacional: hamiltoniano molecular (no relativista), aproximación de Born-Oppenheimer, teoría LCAO (combinación lineal de orbitales atómicos), ecuaciones de Roothaan-Hall (Hartree-Fock), teoría del funcional de la densidad DFT (Kohn-Sham, LDA), métodos post-HF (MP2, CISD, CCSD), descriptores de reactividad (Fukui, dureza/química, índice de electrofilia) y teoría del estado de transición cuántica. Incluye SDFs para orbitales moleculares, densidad electrónica, superficie de energía potencial, enlace químico, correlación electrónica, estados excitados, transferencia de carga, vibraciones moleculares, efecto Jahn-Teller y polarización por solvente.

## Por qué
La química cuántica reduce la química a física aplicada. Esta librería traduce los operadores y métodos de la estructura electrónica a funciones computables y formas visualizables dentro del Lenguaje Hermético.

## Para qué
Modelar estructura electrónica molecular, orbitales moleculares (HOMO/LUMO), superficies de energía potencial, densidad de enlace, correlación electrónica, estados excitados, transferencia de carga, modos vibracionales y efectos de solvente.

## Funciones principales
- **molecular_hamiltonian(e1x..e2z, n1x..n2z, Z₁, Z₂)** — Ĥ = T̂ + V̂ₑₙ + V̂ₑₑ + V̂ₙₙ (2e⁻ + 2 núcleos, escalar; T = 1.0 u.a.)
- **born_oppenheimer_energy(electronic_energy, nuclear_repulsion)** — Separación de movimientos
- **potential_energy_surface(R_eq, k, R)** — PES armónica: E ≈ ½k(R-R_eq)²
- **molecular_orbital_lcao(c₁, c₂, A, B, α, p)** — ψ = c₁φ₁ + c₂φ₂ (gaussianas 1s en 2 centros, punto de evaluación explícito)
- **overlap_integral(α, R)** — S = (π/2α)^{3/2}·e^{-αR²/2} (analítica, gaussianas 1s)
- **hamiltonian_matrix_element(S, H_μμ, H_νν, K)** — Wolfsberg-Helmholtz: H_μν ≈ K·S·(H_μμ+H_νν)/2
- **roothaan_hall_equations(F₁₁, S₁₁, F₂₂, S₂₂)** — ε_i ≈ F_ii/S_ii (diagonal 2×2)
- **electron_density(n₁, n₂, A, B, α, p)** — ρ(r) = Σ n_i|φ_i|²
- **charge_density_difference(ρ_mol, ρ_at1, ρ_at2)** — Δρ = ρ_mol - Σρ_at (2 átomos)
- **slater_determinant(ψ₁e₁, ψ₂e₁, ψ₁e₂, ψ₂e₂, 𝒩)** — det 2×2 / 𝒩 (𝒩 = √N! del llamador)
- **configuration_interaction_coefficient(c, H_IJ)** — ⟨Ψ_I|Ĥ|Ψ_J⟩ ≈ c·H
- **dft_total_energy(ρ, t, v, j, xc)** — E[ρ] ≈ ρ·(t+v+j+xc) (funcionales lineales)
- **hartree_energy(ρ)** — J ≈ ½ρ²
- **lda_exchange_correlation_energy(ρ)** — ε_xc ≈ -0.7386·ρ^{1/3}
- **fukui_function(lumo_density)** — f(r) ≈ |ψ_LUMO(r)|²
- **chemical_hardness(IP, EA)** / **chemical_softness(η)** — η = (IP-EA)/2
- **electrophilicity_index(μ, η)** — ω = μ²/(2η)
- **mp2_correlation_energy(...4 integrales, ε_occ×2, ε_virt×2)** — E_corr MP2 (2×2)
- **cisd_energy(E_ref, c₁, H₁, c₂, H₂)** — E_CISD (2 excitaciones)
- **coupled_cluster_energy(E₀, t₁ᵃ, t₁ᵇ, t₂ᵃ, t₂ᵇ)** — E_CCSD (2 amplitudes cada uno)
- **kohn_sham_band_gap(ε_HOMO, ε_LUMO)** — Δ_KS = ε_LUMO - ε_HOMO

## SDFs definidas
- **molecular_orbitals_visualization(c₁₁, c₁₂, c₂₁, c₂₂, d, intensity_threshold)** — 2 orbitales LCAO (±d/2 en x)
- **molecular_electron_density(n₁xyz, Z₁, n₂xyz, Z₂, intensity_threshold)** — Densidad gaussiana 2 núcleos + patrón de enlace
- **potential_energy_surface_visualization(E_r, E_ts, E_p, intensity_threshold)** — PES radial continua (parábola interpolante)
- **frontier_orbitals_visualization(α, d, intensity_threshold)** — HOMO enlazante (g₁+g₂) / LUMO antienlazante (g₁-g₂)
- **chemical_bond_density(bond_length, bond_order, intensity_threshold)** — Densidad de enlace (enlace en x, ±d/2)
- **electron_correlation_hole(ρ_ref, ρ_corr, intensity_threshold)** — Agujero de correlación
- **excited_states_absorption(E₀, E₁, E₂, E₃, d₁, d₂, d₃, intensity_threshold)** — 3 transiciones lorentzianas
- **charge_transfer_states(donor_density, acceptor_density, coupling, intensity_threshold)** — Transferencia de carga
- **molecular_vibrations_normal_modes(d, ν₁, ν₂, ν₃, intensity_threshold)** — 2 átomos + 3 modos normales
- **jahn_teller_distortion(distortion_strength)** — Estado degenerado → 2 lóbulos al distorsionar
- **solvent_polarization_effect(solute_density, dielectric_constant, intensity_threshold)** — Polarización por solvente

## Notas
- Los cálculos son aproximaciones conceptuales, no sustitutos de paquetes cuánticos reales.
- Las integrales de dos electrones y los elementos de matriz están simplificados.
- Los métodos post-HF (MP2, CISD, CCSD) son representaciones del flujo algorítmico.
- DFT usa funcional LDA; no se implementan GGA ni híbridos.
- Ideal para visualizar conceptos de estructura electrónica en 3D.
- Corregido 2026-10-17: `<` léxico eliminado (PES → `step`); el lenguaje no tiene parámetros función ni arrays: `molecular_hamiltonian` → 2e⁻ + 2 núcleos con `sq` (14 args); LCAO/densidad con punto de evaluación explícito; `overlap_integral` → forma gaussiana analítica (π/2α)^{3/2}·e^{-αR²/2}; `hamiltonian_matrix_element` → Wolfsberg-Helmholtz; `slater_determinant` → det 2×2 con normalización como argumento (`factorial` no existe); MP2/CISD/CCSD desplegados a 2 términos; `for`/`append`/`union_array`/matrices eliminados; primitivas vectoriales → `custom`; `if` en PES → parábola interpolante continua por (0.3; 0.09·E_r), (0.45; E_ts), (0.6; 0.04·E_p); defs sin `let`.
- Verificado 2026-10-17: compila (exit 0); probe 22 nodos / 16 customs exactos; checks: Ĥ(e⁻ en 0.2/0.8 a₀, núcleos 0 y 1 a₀)=−8.8333 Ha, solapamiento S(α=1,0)=1.9687 y S(1,1)=1.1941, PES continuo (0.18/0.06), MP2=−0.0284, determinante Slater=0.693, CCSD=0.125, CISD=0.8, Roothaan=[2.0, 2.667].
- Re-verificado 2026-11-04 (post-fix del resolver): sin cambios en las expresiones emitidas (no había colisiones param/prefijo); **25/25 evaluaciones escalares OK**, incluidos Ĥ(H₂)=−8.833284 Ha, S(1,0)=1.968701, S(1,1)=1.194078, y MP2/CISD/CCSD con los argumentos de prueba actuales. `roothaan_hall_equations` devuelve array y no se evalúa por esta vía.
