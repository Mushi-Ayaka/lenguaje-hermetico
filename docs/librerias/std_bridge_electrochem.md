# std_bridge_electrochem.hm

**Capa:** Intermediario: Física-Química (3.8 - Electroquímica)
**Versión:** 1.0
**Fuente:** Bard & Faulkner "Electrochemical Methods", Bockris & Reddy "Modern Electrochemistry"
**Dependencias:** std_math_core.hm
**Fecha:** 6 de junio de 2026

## Cómo
Define constantes electroquímicas (F, R, RT, k_BT/e, potenciales estándar de electrodo) y funciones de termodinámica redox (Nernst, ΔG, K_eq, FEM de celda), cinética electroquímica (Butler-Volmer, Tafel, Levich, Koutecký-Levich, Randles-Ševčík), interfases (Helmholtz, Gouy-Chapman, Poisson-Boltzmann), conductividad iónica (Kohlrausch, Einstein, Stokes-Einstein), baterías (capacidad específica, energía, potencia, eficiencia coulombica), corrosión (potencial de corrosión, velocidad, protección catódica) y electrodos especiales (Henderson, ISE, pH). Incluye SDFs para celdas electroquímicas, baterías (Li-ion, Pb-ácido), celdas de combustible (PEM), celdas clorálcali, corrosión localizada y voltametría cíclica.

## Por qué
La electroquímica gobierna desde las baterías hasta la corrosión. Esta librería codifica las ecuaciones que describen el potencial, la corriente y el transporte iónico en interfaces y celdas.

## Para qué
Modelar potenciales de electrodo (Nernst), cinética de transferencia de carga (Butler-Volmer), voltametría cíclica, doble capa electroquímica, conductividad de electrolitos, baterías (Li-ion, Pb-ácido), celdas de combustible, corrosión y electrodos selectivos de iones.

## Funciones principales
- **nernst_equation(E₀, n, a_ox, a_red)** — E = Eº - (RT/nF)·ln(Q)
- **cell_emf(E_cathode, E_anode)** — FEM = E_cátodo - E_ánodo
- **galvanic_cell_potential(E₀_cell, n, Q)** — Potencial de celda con concentración
- **butler_volmer_current(i₀, α_transfer, n, η)** — i = i₀[exp(αnFη/RT) - exp(-(1-α)nFη/RT)]
- **tafel_equation(i, i₀, α_transfer, n)** — η = a + b·log|i|
- **randles_sevcik_current(n, A, C, D, scan_rate)** — i_p ∝ √(scan_rate) (voltametría cíclica)
- **kohlrausch_conductivity(Λ₀, K, c)** — Λ = Λ₀ - K√c
- **ionic_mobility_einstein(z, D, T)** — u = zeD/k_BT
- **specific_capacity(n, molar_mass)** — Capacidad en mAh/g
- **specific_energy(capacity_mAh_g, voltage)** — Energía específica en Wh/kg
- **coulombic_efficiency(Q_discharge, Q_charge)** — η_C = Q_desc/Q_carga
- **corrosion_potential(E₀_a, i₀_a, α_a, E₀_c, i₀_c, α_c, n)** — Intersección Tafel
- **pH_electrode_response(E₀, pH)** — E = E₀ + (RT·ln10/F)·pH
- **liquid_junction_potential(t⁺, t⁻, a₁, a₂)** — Potencial de unión líquida (Henderson)
- **ion_selective_electrode_potential(E₀, z, a_primary, k1, a_j1, k2, a_j2)** — ISE con 2 interferentes (Σ desplegada)

## SDFs definidas
- **nernst_potential_surface(E₀, n, a_ox, a_red, potential_value)** — Superficie isopotencial de Nernst
- **butler_volmer_current_field(i₀, α, n, overpotential_field, current_threshold)** — Campo de corriente 3D
- **electrode_electrolyte_interface(ψ₀, κ, interface_thickness)** — Doble capa (Poisson-Boltzmann)
- **electrochemical_cell(E₀_anode, E₀_cathode, n, separator_thickness, electrode_radius)** — Celda ánodo-cátodo
- **lithium_ion_battery(num_layers, layer_thickness, current_collector_thickness)** — Batería Li-ion multicapa
- **localized_corrosion_pit(pit_depth, pit_radius, corrosion_rate)** — Picadura de corrosión
- **cyclic_voltammogram_3d(E₀, n, A, C, D, scan_rate, potential_range, current_threshold)** — Voltamograma cíclico
- **hydrogen_fuel_cell(membrane_thickness, catalyst_thickness, flow_field_pattern)** — Celda PEM
- **lead_acid_battery(plate_thickness, separator_thickness, electrolyte_level)** — Batería Pb-ácido
- **chlor_alkali_cell(anode_material, cathode_material, membrane_type, cell_voltage)** — Celda clorálcali

## Notas
- Constantes escaladas para visualización; los potenciales están en voltios escalados.
- Las corrientes son relativas, no absolutas.
- Los potenciales estándar incluidos (H₂, O₂, Zn, Cu, Ag, Li) son valores reales vs SHE.
- Las geometrías de baterías y celdas son representaciones simplificadas.
- Incluye cinética de corrosión con protección catódica.
- Corregido 2026-10-17: `alpha` es keyword (no puede ser nombre de parámetro) → `alpha_transfer` en Butler-Volmer/Tafel/sobretención; sin arrays/`for` (ISE desplegado a 2 interferentes); los 11 defs sin `let`/`fn` anidada: `box` toma 3 half-extents escalares centrados en origen y `sphere`/`cylinder` también, así que las geometrías posicionadas usan `custom` con coordenadas relativas; uniones binarias anidadas; fases de burbujas Cl₂/H₂ distintas (π/2 vs π).
