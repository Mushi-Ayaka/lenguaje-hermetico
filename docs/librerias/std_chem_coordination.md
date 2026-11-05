# std_chem_coordination.hm

**Capa:** Química (4.3 — Química de Coordinación)
**Versión:** 1.1
**Fuente:** "Coordination Chemistry" by Cotton & Wilkinson
**Dependencias:** std_math_core.hm, std_bridge_atomic.hm
**Fecha:** 6 de junio de 2026 (reescrita 2026-10-17)

## Cómo
Implementa teoría de complejos de coordinación: clasificación de ligandos, serie espectroquímica, número de coordinación, teoría del campo cristalino (desdoblamiento octaédrico/tetraédrico, energía de estabilización, predicción alto/bajo espín), propiedades magnéticas (momento magnético, susceptibilidad de Curie), espectros electrónicos (transiciones d-d, predicción de color, reglas de selección de Laporte), y SDFs de complejos (octaédrico, tetraédrico, cuadrado plano, campo cristalino visualizado, quelación bidentada).

El lenguaje no tiene strings, arrays, `for` ni `if`: todas las funciones devuelven **códigos numéricos** documentados, y las selecciones se hacen con `step`/`select` (condiciones >= 0.5).

## Por qué
Los complejos de coordinación son centrales en química inorgánica, catálisis, magnetismo y espectroscopia. Explica por qué los complejos tienen color, magnetismo y geometrías específicas.

## Para qué
- Clasificar ligandos por denticidad, hapticidad y fuerza (serie espectroquímica)
- Predecir geometría por número de coordinación
- Calcular desdoblamiento del campo cristalino (Δ_oct, Δ_tet)
- Determinar configuraciones de alto/bajo espín
- Estimar momento magnético efectivo y susceptibilidad
- Predecir color de complejos por transiciones d-d
- Visualizar complejos y desdoblamiento de orbitales d

## Convenciones numéricas
| Parámetro | Valores |
|---|---|
| `ligand_index` | 0=I⁻ 1=Br⁻ 2=Cl⁻ 3=F⁻ 4=OH⁻ 5=H₂O 6=NH₃ 7=en 8=NO₂⁻ 9=CN⁻ 10=CO |
| `metal_index` | 0=Ti 1=V 2=Cr 3=Mn 4=Fe 5=Co 6=Ni 7=Cu 8=otro |
| `geometry_index` | 0=octaédrico 1=tetraédrico (códigos de geometría: ver `coordination_number_geometry`) |
| `field_strength` | 0=campo débil (alto espín) 1=campo fuerte (bajo espín) |

## Funciones principales
| Función | Devuelve | Descripción |
|---|---|---|
| `ligand_classification(donor_atoms, hapticity, bite_angle)` | valor codificado | entero = denticidad (1–4); décimas = η−1; centésimas = bite/180 si quelante |
| `ligand_field_strength(ligand_index)` | 0.1–0.9 | Fuerza de ligando (serie espectroquímica) |
| `coordination_number_geometry(coordination_number, strong_field)` | código | 2=lineal 3=trigonal_planar 4=tetraédrico 5=bipirámide_trigonal 6=octaédrico 7=bipirámide_pentagonal 8=antiprisma_cuadrado; +30 si CN=4 y campo fuerte (34=cuadrado_plano) |
| `crystal_field_splitting_octahedral(metal_index, oxidation_state, average_ligand_strength)` | cm⁻¹ | Δ_oct = base(metal, ox)·strength·2 |
| `crystal_field_splitting_tetrahedral(metal_index, oxidation_state, average_ligand_strength)` | cm⁻¹ | Δ_tet = (4/9)·Δ_oct |
| `crystal_field_stabilization_energy(electron_count, geometry_index, splitting, pairing_energy)` | cm⁻¹ | EECC = (0.4·n_t2g−0.6·n_eg)·Δ − n_pair·P |
| `high_spin_low_spin_prediction(splitting_energy, pairing_energy, electron_count)` | código | 0=config única (d0–d3, d8–d10) 1=bajo espín (Δ>P) 2=alto espín (Δ<P) |
| `effective_magnetic_moment(unpaired_electrons, spin_only)` | μ_B | √(n(n+2)); ×1.05 si spin_only=0 |
| `magnetic_susceptibility(magnetic_moment, temperature)` | χ | Ley de Curie (SI) con AVOGADRO_NUMBER y BOLTZMANN_CONSTANT |
| `unpaired_electron_count(metal_index, oxidation_state, geometry_index, field_strength)` | n | d = 4+metal_index−ox; config. alto/bajo espín por campo |
| `dd_transition_energies(d_electron_count, geometry_index, splitting)` | cm⁻¹ | transición más baja: d²→0.8Δ, d³/d⁸→0.9Δ (oct) |
| `complex_color_prediction(absorption_wavelength)` | código | 0=incoloro(UV) 1=amarillo 2=naranja 3=rojo 4=púrpura 5=azul 6=verde 7=incoloro(IR) |
| `laporte_selection_rule_allowed(initial_orbital, final_orbital, centrosymmetric)` | 1/0 | 0=t2g 1=eg; prohibida g→g en complejos centrosimétricos |

## SDFs definidas (centradas en el origen; usar `position:` en la instancia)
| SDF | Descripción |
|---|---|
| `octahedral_complex(ligand_distance, metal_radius, ligand_radius, bond_radius)` | Metal + 6 ligandos ±d + 6 enlaces cápsula + 3 anillos t2g (planos xy/xz/yz) + 4 lóbulos eg (±x, ±y) |
| `tetrahedral_complex(ligand_distance, metal_radius, ligand_radius, bond_radius)` | Metal + 4 ligandos en (1,1,1)/√3 + 4 enlaces |
| `square_planar_complex(ligand_distance, metal_radius, ligand_radius, bond_radius)` | Metal + 4 ligandos en plano xz + 4 enlaces + 4 lóbulos dx²−y² |
| `crystal_field_splitting_visualization(t2g_electrons, eg_electrons)` | Líneas t2g/eg + electrones (spin alternado) + flecha Δ |
| `chelation_bidentate_ligand(metal_radius, bite_angle)` | Metal + 2 donadores + puente + arco de mordida (bite_angle en grados) |

## Corregido 2026-10-17
- **Reescritura completa** (sintaxis v0.18): se eliminaron strings, arrays (`ligand_types[...]`, `electron_config[...]`), `for`, `if/else`, `||`/`&&`, `append`, `len`, `><` comparadores y primitivas vectoriales inexistentes (`torus([vec],...)`, `cylinder([vec],[vec],r)`, `sphere([vec],r)`).
- Firmas escalarizadas: metal y ligandos ahora por índice numérico; configuraciones electrónicas como conteos; `average_ligand_strength` pasa el promedio de `ligand_field_strength` (el llamador lo calcula).
- Fórmulas: `d^n = 4 + metal_index − oxidation_state` (antes `ox + 2 + mi`, incorrecto). EECC corregido por geometría (oct: 3 t2g abajo; tet: invertido). Predicción alto/bajo espín con `select` y umbral Δ≥P.
- SDFs reconstruidos con `custom(min-chain)` plano + `segment_distance` (std_bridge_atomic) para enlaces; todo centrado en origen.
- Eliminados parámetros muertos (`threshold`, `splitting_energy`) y constantes no usadas (PLANCK/BOLTZMANN propias → `BOLTZMANN_CONSTANT` de math_core).

## Verificado 2026-10-17
- Compila: exit 0; probes con 5 SDFs (instancias) y 16 fns (como `metal_radius`) → exit 0; `.rih`: 9 customs en probe1 (2+1+2+2+2) y 32 en probe2, sin `for`/`if`/`let`/`atan2` residuales.
- 44/44 checks numéricos OK (`coord_checks.ps1`): p. ej. splitting_oct(Cr³⁺, 0.4)=17200, Δ_tet=7644.44, EECC(d⁴, oct, 20000, 15000)=17000, μ(BM, n=3)=√15=3.873, χ(300 K)=0.062525, unpaired(Fe²⁺, débil)=4 / (fuerte)=0, ligand_field_strength(10)=0.9, geometría CN=4 fuerte=34, color(650 nm)=6 (rojo absorbido→verde), Laporte (t2g→eg, centro)=0, ligand_classification(4,1,180)=4.01.
- SDFs: puntos verificados en nodos octahedral/tetrahedral/square_planar/viz/chelation (metal −r, ligandos −r, enlaces −r, anillos −0.008, lóbulos −0.02, líneas t2g/eg −0.01, electrones −0.015, arco −0.003; exteriores > 0). Electrones apagados: sdf = 0 exacto (sin interior, invisible).
- Sin artefactos residuales (probes y `.rih`/`.ont`/`.obs` eliminados).

## Notas
V1.1. Las transiciones d-d devuelven solo la transición más baja (d², d³, d⁸ octaédricas). El momento magnético usa aproximación spin-only con factor orbital opcional. `magnetic_susceptibility` usa constantes de `std_math_core.hm` (AVOGADRO_NUMBER, BOLTZMANN_CONSTANT) y `BOHR_MAGNETON_COORD` local.
