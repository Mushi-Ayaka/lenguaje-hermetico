# std_chem_inorganic.hm

**Capa:** Química (4.1 — Química Inorgánica)
**Versión:** 1.1
**Fuente:** "Inorganic Chemistry" by Shriver & Atkins
**Dependencias:** std_math_core.hm, std_bridge_atomic.hm
**Fecha:** 6 de junio de 2026 (reescrita 2026-10-23)

## Cómo
Proporciona un ecosistema de química inorgánica: configuración electrónica, tabla periódica (periodo y grupo por Z), tendencias periódicas (radio, ionización, electronegatividad, afinidad electrónica), enlaces inorgánicos (iónico, metálico, covalente), teoría VSEPR con códigos de geometría y distorsión por pares solitarios, estados de oxidación, compuestos inorgánicos (óxidos, haluros, calcogenuros, oxoaniones) y SDFs estructurales (NaCl, fluorita, semiconductores, vidrio, cerámica, catalizadores).

El lenguaje no tiene strings, arrays, `for` ni `if`: todas las funciones devuelven **códigos numéricos** documentados y las selecciones se hacen con `step`/`select` (condiciones ≥ 0.5).

## Por qué
Cubre la química de elementos no orgánicos: metales, minerales, materiales, semiconductores y catalizadores. Fundamental para ciencia de materiales.

## Para qué
- Obtener periodo, grupo y conteo de electrones (valencia/core) desde Z
- Estimar tendencias periódicas y energías de enlace
- Predecir geometrías VSEPR (2–12 dominios) con distorsión por pares solitarios
- Asignar estados de oxidación comunes y su estabilidad
- Clasificar óxidos como ácidos/básicos/anfóteros
- Generar SDFs de estructuras cristalinas y materiales inorgánicos

## Convenciones numéricas
| Parámetro | Valores |
|---|---|
| `vsepr_geometry` | 0=desconocida 2=lineal 3=trigonal plana 4=tetraédrica 5=bipirámide trigonal 6=octaédrica 7=angular 8=piramidal trigonal 9=balancín 10=forma T 11=piramidal cuadrada 12=cuadrada plana |
| `oxide_character` | 1=ácido (Δχ>1.7) 2=básico (Δχ<1.0) 3=anfótero (1.0≤Δχ≤1.7) |
| `period_of` | 1–7 (por Z) |
| `group_of` | 1–18 (lantánidos/actínidos → 3) |
| `electron_pairs`/`lone_pairs` | dominios de enlace / pares libres (t = suma) |

## Funciones principales
| Función | Devuelve | Descripción |
|---|---|---|
| `core_electron_count(Z)` | electrones | Electrones de capas internas (2, 10, 18, 36, 54, 86) |
| `period_of(Z)` | 1–7 | Periodo por umbrales 2/10/18/36/54/86 |
| `group_of(Z)` | 1–18 | Grupo; f-block → 3 |
| `valence_electron_count(Z)` | electrones | Z − core |
| `atomic_radius_periodic(period, group)` | longitud | 0.529·n²/(2+0.5g+0.3p) (≈Å) |
| `ionization_energy_periodic(Z, degree)` | eV | 13.6·z_eff²/n²; base = 0.65 + 1.6(1−e^(−0.25·val)) + 0.3(n−1); z_eff = base + 0.8(degree−1) |
| `pauling_electronegativity(Z)` | 0.7–4.0 | χ = 0.9 + 0.15g − 0.08p (H = 2.2) |
| `electron_affinity_periodic(Z)` | eV | −110·χ (0 en gases nobles) |
| `ionic_lattice_energy(z_cation, z_anion, r_cation, r_anion, madelung)` | unidades relativas | M·z⁺z⁻/(r⁺+r⁻)·0.9 |
| `metallic_bond_energy(density, fermi_energy)` | unidades relativas | 0.6·ρ·E_F |
| `covalent_bond_strength(overlap, bond_order)` | unidades relativas | overlap·order·100 |
| `percent_ionic_character(delta_en)` | % | (1−e^(−Δχ²/4))·100 |
| `bond_length_covalent(r1, r2, bond_order)` | longitud | r1 + r2 − 0.07·ln(order) |
| `vsepr_geometry(electron_pairs, lone_pairs)` | código | t = e+l; ver convenciones |
| `bond_angles_vsepr(geometry_code)` | grados | 2→180 3→120 4→109.47 5→120 6→90 7→104.5 8→107 9→102 10→90 11→85 12→90 |
| `lone_pair_distortion(ideal_angle, lone_pairs)` | grados | ideal − 2.5·lone |
| `common_oxidation_state(group)` | estado | g≤8→g; 9→6; 10→4; 11→3; 12→2; 13→3; 14→4; 15→5; 16→6; 17→7; 18→0 |
| `standard_reduction_potential(Z, ox_high, ox_low)` | V (aprox.) | 0.5(χ−1.5) + 0.1(ox_high−ox_low) |
| `oxidation_state_stability(Z, oxidation_state)` | 0–1 | v = val−ox; d = min(\|v\|, \|v−8\|, \|v−10\|, \|v−18\|); 1−d/9 |
| `oxide_character(en_metal, en_oxygen)` | código | 1=ácido 2=básico 3=anfótero |
| `halide_stability(charge, ionic_ratio, size_factor)` | unidades | q/(r+1e−9)·n·0.1 |
| `chalcogenide_band_gap(Z_metal, Z_chalcogen)` | eV (aprox.) | 2·\|χA−χB\| |
| `oxoanion_geometry(oxygen_count, charge)` | código | O≥6→6; O≥4→4; O=3 con carga≤−2.5→3 (plana); O=3 con carga mayor→8 (piramidal); resto→0 |

## SDFs definidas (centradas en el origen; usar `position:` en la instancia)
| SDF | Descripción |
|---|---|
| `nacl_crystal_structure(lattice_constant)` | Red rocksalt: 8 iones (Na r=0.22a, Cl r=0.28a) en esquinas y centros de cara |
| `vsepr_molecular_geometry(bond_pairs, lone_pairs)` | Central + hasta 6 átomos + hasta 3 nubes de par libre; posiciones reales (lineal, trigonal, tetraédrica, bipirámide trigonal, octaédrica); ítems apagados colapsan al centro (gates step) |
| `octahedral_coordination_complex(ligand_distance, metal_radius, ligand_radius, bond_radius)` | Metal + 6 ligandos sobre ejes + 6 enlaces cápsula (`segment_distance`) |
| `metal_oxide_layer(metal_thickness, oxide_thickness, half_width)` | `union(box, custom)`: losa metálica + lámina de óxido encima |
| `fluorite_crystal_structure(lattice_constant)` | CaF₂: 8 Ca en esquinas (r=0.16a) + 8 F en (¼,¼,¼) (r=0.10a) |
| `inorganic_semiconductor_structure(lattice_constant, band_gap)` | Zinc-blenda (8 esferas) + placa inferior de espesor 0.005+0.01·gap |
| `inorganic_glass_silica(disorder_factor)` | SiO₂ amorfo: 5 Si + 5 O desplazados 0.08 en x con corrimiento 0.05·(a·cosφ + b·sinφ) |
| `ceramic_alumina_structure(grain_size, porosity)` | `subtract`: 4 granos Al₂O₃ − 4 poros de radio 0.0005+0.15·g·porosity |
| `inorganic_catalyst_surface(active_sites)` | Soporte (box) + hasta 6 sitios activos + hasta 3 adsorbatos, activados por gates `step(k−0.5, sites)` |

## Corregido 2026-10-23
- **Reescritura completa** (sintaxis v0.18): eliminados strings, arrays, `for`, `if/else`, `>`, `union_array` y primitivas vectoriales inexistentes (`sphere([vec],r)`, `cylinder([vec],[vec],r)`, `torus`).
- Tabla periódica reconstruida: `period_of` con umbrales reales; `group_of` sin `select` (compuertas `step` anidadas, f-block → 3, correcto en H, He, Fe, Cu, Zn, Br, Kr, Xe, Hf, Rn, Fr); `core_electron_count` y valencia coherentes.
- Corregidas fórmulas clave: `vsepr_geometry` usa t = e+l y códigos 2–12 (AX₄E→9, AX₄E₂→12); `oxide_character` con umbrales de Pauling; `chalcogenide_band_gap` = 2Δχ (antes tabla fija); `oxoanion_geometry` según conteo de O y carga; `standard_reduction_potential` y `oxidation_state_stability` derivadas de χ y valencia.
- SDFs reconstruidos con `custom(min-chain)` plano, `union`/`subtract` binarios y `segment_distance`; gates `step` para sitios/vacantes y anti point-zero (posición escalada por el gate).
- Eliminados parámetros muertos (`threshold`, `type`, `center`, `density`) y constantes de módulo no usadas (`AVOGADRO_INORG`, `FARADAY_INORG`, `BOHR_RADIUS_INORG`).
- Dependencias reducidas a `std_math_core.hm` + `std_bridge_atomic.hm` (la v1.0 listaba `std_chem_crystal.hm`, inexistente).

## Verificado 2026-10-23
- Compila: exit 0 (`libreria/std_chem_inorganic.hm`). Probes: 9 defs con args numéricos (7 s) y 12 instancias con las 23 fns vía `def chk4` (sphere params); `.rih`: 11 customs (1+1+1+1+1+2+1+2+1), sin `for`/`if`/`let`/`atan2`/`union_array`/`append`/`?` residuales.
- **98/98 checks OK** (`inorg_checks.ps1`): 48 valores de fns (p. ej. `period_of(26)=4`, `group_of(26)=8`, `group_of(72)=4`, `group_of(54)=18`, `valence(26)=8`, `atomic_radius(4,8)=1.17556`, `ionization(11,1)=3.8874`, `χ(26)=1.78`, `χ(1)=2.2`, `EA(17)=−353.1`, `lattice(1,1,0.1,0.2,1.75)=5.25`, `percent_ionic(2)=63.212`, `bond_length(0.1,0.1,2)=0.15148`, `vsepr(4,1)=9`, `vsepr(4,2)=12`, `vsepr(6,1)=11`, `vsepr(2,1)=7`, `bond_angles(8)=107`, `distortion(109.47,1)=106.97`, `ox_state(8/17/11/18)=8/7/3/0`, `E°(Fe,3,2)=0.24`, `stability(26,2)=0.7778`, `halide(2,3,0.1)=6`, `gap(30,16)=1.36`, `oxoanion(3,−3)=8`, `(4,−2)=4`, `(6,−3)=6`, `oxide(3.5,3.5)=2`, `oxide(1.0,3.5)=1`, `oxide(2.5,3.5)=3`).
- **33 puntos SDF** verificados (interiores negativos: iones NaCl −0.11/−0.14, central VSEPR −0.045, metal complejo −0.08 y enlaces −0.012, losa metálica −0.1 y óxido −0.02, Ca −0.096 y F −0.06, catión/anion semiconductor −0.065 y placa −0.0093, Si −0.035 y O −0.022, grano alúmina −0.06, soporte catalizador −0.02 y sitios/adsorbatos; exteriores > 0; poro de alúmina +0.0305).
- **8 compuertas** verificadas: `vsepr(2,1)` apaga A3/A5 y conserva nube libre (−0.055); `catalyst(2)` apaga sitio 3 y adsorbato 2.
- Sin artefactos residuales (probes y `.rih` eliminados).

## Notas
V1.1. Los modelos son aproximaciones de tendencias (no ab initio). Pasar llamadas a fns como argumentos de SDFs cuyos parámetros se repiten es costoso por la sustitución textual del resolver (p. ej. `nacl_crystal_structure(group_of(11))` ≈ 21 s y params de ~100 KB en `.rih`); se recomienda precalcular y pasar valores numéricos. `group_of` devuelve 3 para lantánidos y actínidos por simplicidad.
