# std_bridge_atomic.hm

**Capa:** Intermediario: Física-Química (3.1/3.2 - Física Atómica y Molecular)
**Versión:** 1.0
**Fuente:** Bohr "Atomic Structure", Schrödinger "Wave Mechanics"
**Dependencias:** std_math_core.hm
**Fecha:** 6 de junio de 2026

## Cómo
Provee constantes atómicas fundamentales escaladas (h, m_e, e, k_B, R_∞, a₀, α), funciones del modelo de Bohr (radios, velocidades, energías, transiciones), funciones de onda orbitales hidrogenoides (1s, 2s, 2p_z, 3d_z²), y SDFs para visualizar orbitales, capas electrónicas, nubes de densidad y moléculas (H₂O, CH₄, diatómica). Incluye potenciales de enlace (Morse, Lennard-Jones) y perfiles de línea espectral (Lorentz, Gauss, Voigt).

## Por qué
La estructura atómica y los enlaces químicos son el punto de partida para cualquier modelo molecular. Esta librería traduce la física cuántica del átomo a formas geométricas y funciones numéricas que el Lenguaje Hermético puede representar.

## Para qué
Modelar y visualizar átomos hidrogenoides, orbitales atómicos, enlaces covalentes (σ y π), moléculas simples, espectros de emisión/absorción atómicos y líneas espectrales en 3D.

## Funciones principales
- **bohr_orbit_radius(n, Z)** — Radio de la órbita n-ésima: rₙ = n²·a₀/Z
- **bohr_energy_level(n, Z)** — Energía del nivel n: Eₙ = -13.6·Z²/n² eV
- **bohr_transition_wavelength(n₁, n₂, Z)** — Longitud de onda de transición
- **orbital_1s_wavefunction(Z, r)** / **orbital_2s_wavefunction(Z, r)** / **orbital_2pz_wavefunction(Z, x, y, z)** / **orbital_3dz2_wavefunction(Z, x, y, z)** — Funciones de onda hidrogenoides
- **sigma_bond_orbital(Z, xA, yA, zA, xB, yB, zB, x, y, z)** — Orbital molecular σ (LCAO)
- **pi_bond_orbital(...)** — Orbital molecular π
- **morse_potential(De, a, re, r)** / **lennard_jones_potential(epsilon, sigma, r)** — Potenciales de enlace
- **spectral_line_intensity(n₁, l₁, n₂, l₂)** — Intensidad con regla de selección Δl = ±1
- **segment_distance(px, py, pz, ax, ay, az, bx, by, bz)** — Distancia punto-segmento AB (helper para enlaces cápsula)
- **voigt_profile_approx(nu, nu₀, gamma, sigma)** — Perfil de línea espectral

## SDFs definidas
- **orbital_1s_surface(Z, probability_threshold)** — Esfera de probabilidad 1s
- **orbital_2s_surface(Z, probability_threshold)** — Esfera con nodo radial
- **orbital_2pz_surface(Z, probability_threshold)** — Forma de dumbbell (lóbulo)
- **orbital_3dz2_surface(Z, probability_threshold)** — Forma de rosquilla
- **complete_electron_cloud(Z, total_density_threshold)** — Nube electrónica completa
- **electron_shells(Z, shell_radius_factor)** — Capas K, L, M
- **diatomic_molecule(Z, bond_length, atom_radius)** — Molécula diatómica (H₂, O₂, N₂)
- **water_molecule(O_radius, H_radius, bond_length, bond_angle_deg)** — H₂O
- **methane_molecule(C_radius, H_radius, bond_length)** — CH₄ tetraédrica
- **sigma_bond_visualization(Z, xA, yA, zA, xB, yB, zB, density_threshold)** — Orbital σ 3D
- **morse_potential_surface(De, a, re, potential_value)** — Superficie equipotencial de Morse
- **balmer_spectral_lines(Z, intensity_threshold)** — Serie de Balmer (Hα, Hβ, Hγ)
- **spectral_line_profile_3d(nu₀, gamma, sigma, intensity_threshold)** — Perfil de línea 3D
- **atomic_emission_spectrum(Z, T, intensity_threshold)** — Espectro de emisión atómica

## Notas
- Constantes atómicas escaladas para uso visual (no son valores reales del SI).
- Orbitales atómicos aproximados, no soluciones exactas de Hartree-Fock.
- Moléculas modeladas geométricamente, sin campos de fuerza ab initio.
- Potenciales de enlace (Morse, LJ) son aproximaciones clásicas.
- Espectros son representaciones visuales, no simulaciones espectroscópicas precisas.
- Corregido 2026-10-17: los `def` no aceptan `let` (reescritos con expresiones inline y helpers `fn`), `union` es binario (anidado), `sphere(r)` solo centra en el origen (átomos posicionados vía `custom`), `atan2` → `atan(y, x)`, y los enlaces moleculares ahora son cápsulas reales vía `segment_distance`.
