# std_bridge_spectral.hm

**Capa:** Intermediario: Física-Química (3.5 - Espectroscopia Molecular)
**Versión:** 1.0
**Fuente:** Herzberg "Molecular Spectra and Molecular Structure"
**Dependencias:** std_math_core.hm, std_math_algebra.hm, std_math_calculus.hm, std_bridge_atomic.hm, std_phys_relativity.hm
**Fecha:** 6 de junio de 2026

## Cómo
Provee constantes espectroscópicas (B₀, ωₑ, ωₑχₑ, αₑ, R∞) y funciones para espectros rotacionales (rotor rígido, distorsión centrífuga, constante B_v), vibracionales (oscilador armónico y anarmónico), vibración-rotación (ramas R y P) y electrónicos (transiciones, Franck-Condon, progresiones vibracionales). Implementa ley de Beer-Lambert, coeficiente de extinción espectral, emisión térmica y fluorescencia con desplazamiento Stokes. Incluye SDFs para espectros 3D de moléculas específicas (CO, H₂O, CO₂).

## Por qué
Cada molécula tiene una firma espectral única. Esta librería permite sintetizar y visualizar espectros rotacionales, vibracionales y electrónicos a partir de constantes espectroscópicas, conectando la estructura molecular con su huella óptica.

## Para qué
Generar espectros rotacionales puros, vibración-rotación (ramas R/P), progresiones vibracionales electrónicas, espectros de absorción/emisión, fluorescencia y espectros moleculares característicos (CO, H₂O, CO₂).

## Funciones principales
- **rotational_energy_level(B, J)** — E_J = B·J(J+1)
- **rotational_transition_frequency(B, J_lower)** — ΔE = 2B(J+1)
- **pure_rotational_spectrum_intensity(ν, B, T, J_max)** — Intensidad rotacional con degeneración y Boltzmann
- **rotational_energy_with_centrifugal(B, D, J)** — E_J = B·J(J+1) - D·[J(J+1)]²
- **vibration_dependent_rotational_constant(Bₑ, αₑ, v)** — B_v = Bₑ - αₑ(v+½)
- **harmonic_oscillator_energy(ωₑ, v)** — E_v = ωₑ(v+½)
- **anharmonic_oscillator_energy(ωₑ, ωₑχₑ, v)** — E_v = ωₑ(v+½) - ωₑχₑ(v+½)²
- **vibrational_transition_frequency(ωₑ, ωₑχₑ, v_lower, Δv)** — Frecuencia de banda
- **vibration_rotation_R_branch / P_branch(ν₀, B', B", J)** — Ramas R y P
- **vibration_rotation_energy(ωₑ, ωₑχₑ, B_v, D_v, v, J)** — Nivel combinado
- **electronic_transition_frequency(Tₑ, Tₑ", ωₑ', ωₑ", B', B", v', v", J', J")** — Transición completa
- **frank_condon_factor(v', v", displacement)** — |⟨χ_v'|χ_v"⟩|²
- **beer_lambert_absorption(I₀, ε, c, l)** — I = I₀·exp(-εcl)
- **spectral_extinction_coefficient(ν, ν₀, ε_max, γ, profile_type)** — Perfil Lorentz/Gauss mixto
- **thermal_emission_spectrum(ν, T, ν₀, ε_max, γ)** — ε(ν)·B_ν(T)
- **fluorescence_spectrum(ν, ν_exc, Stokes_shift, width)** — Espectro de fluorescencia

## SDFs definidas
- **rotational_spectrum_3d(B, T, J_max, intensity_threshold)** — Líneas rotacionales 3D
- **vibrational_spectrum_3d(ωₑ, ωₑχₑ, T, v_max, intensity_threshold)** — Bandas vibracionales
- **vibration_rotation_spectrum_3d(ν₀, B', B", T, J_max, intensity_threshold)** — Ramas R y P
- **vibrational_progression_3d(Tₑ, ωₑ, ωₑχₑ, B, displacement, T, v_max, intensity_threshold)** — Progresión Franck-Condon
- **absorption_spectrum_3d(ν₀, ε_max, γ, c, l, I₀, intensity_threshold)** — Absorción Beer-Lambert
- **thermal_emission_line_3d(ν₀, ε_max, γ, T, intensity_threshold)** — Emisión térmica
- **CO_molecular_spectrum(T, intensity_threshold)** — CO (B₀=1.93 cm⁻¹, ωₑ=2170 cm⁻¹)
- **H2O_molecular_spectrum(T, intensity_threshold)** — H₂O (ν₁=3657, ν₂=1595, ν₃=3756 cm⁻¹)
- **CO2_molecular_spectrum(T, intensity_threshold)** — CO₂ (ν₁=667, ν₂=2349 cm⁻¹)

## Notas
- Frecuencias en unidades escaladas (no cm⁻¹ reales).
- Intensidades relativas, diseñadas para visualización no para análisis cuantitativo.
- Espectros 3D usan eje x para frecuencia y eje y para intensidad.
- Constantes espectroscópicas basadas en valores típicos de moléculas reales.
- Moléculas específicas (CO, H₂O, CO₂) incluyen sus constantes reales escaladas.
