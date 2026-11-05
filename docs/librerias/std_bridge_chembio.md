# std_bridge_chembio.hm

**Capa:** Intermediario Química-Biología (5.4 — Biología Química)
**Versión:** 1.0
**Fuente:** "Chemical Biology" by Schreiber, Kapoor & Wess
**Dependencias:** std_math_core.hm, std_bridge_atomic.hm
**Fecha:** 6 de junio de 2026

## Cómo
Proporciona funciones para sondas moleculares (eficiencia fluorescente, FRET, fotoblanqueo, Stokes shift), inhibidores (IC50, selectividad, ventana terapéutica, ligand efficiency, LLE), química combinatoria (tamaño de librería, diversidad molecular, Tanimoto, cobertura de espacio químico), actividad biológica (QSAR, regla de Lipinski, fracción sp3, drug-likeness), y conjugación (eficiencia, F/P ratio, grado de sustitución, retención de actividad).

## Por qué
Aplicar herramientas químicas sintéticas y analíticas a sistemas biológicos para diseñar sondas, inhibidores y moléculas bioactivas.

## Para qué
Usada para modelar herramientas de biología química: fluoróforos, FRET, inhibidores enzimáticos, librerías combinatorias, predicción QSAR, diseño de fármacos, y bioconjugación.

## Funciones principales
- `fluorescent_probe_efficiency(quantum_yield, extinction)` — Brillo relativo = Φ·ε
- `fret_efficiency(donor_emission, acceptor_absorption, distance, R0)` — Eficiencia FRET (ε=1e-12)
- `photobleaching_rate(intensity, photosensitizer, oxygen)` — Tasa de fotoblanqueo
- `competitive_inhibition_ic50(ki, S, km)` — IC50 = Ki·(1 + [S]/Km)
- `selectivity_index(ic50_target, ic50_offtarget)` — Índice de selectividad
- `therapeutic_window(toxic_dose, effective_dose)` — Ventana terapéutica
- `ligand_efficiency(binding_energy_J, heavy_atom_count)` — LE = −ΔG/(N·RT), adimensional
- `lipophilic_ligand_efficiency(pic50, logP)` — LLE
- `combinatorial_library_size(blocks, positions)` — Tamaño de librería
- `molecular_diversity(similarity, threshold)` — Diversidad (baja similitud = alta diversidad)
- `chemical_space_coverage(lx, ly, r1x, r1y, r2x, r2y, radius)` — Cobertura de 1 compuesto vs 2 puntos
- `tanimoto_similarity(fp1_bit0, fp1_bit1, fp2_bit0, fp2_bit1)` — Tanimoto escalar (2 bits)
- `qsar_prediction(d1, d2, c1, c2, intercept)` — QSAR lineal (2 descriptores)
- `lipinski_rule_of_five(MW, logP, HBD, HBA)` — Regla de los 5 (nº de violaciones)
- `druglikeness_score(MW, logP, HBD, HBA, rot, aromatic)` — Score de fármaco
- `conjugation_efficiency(reactant, product, time)` — Eficiencia de conjugación

## SDFs definidas
- `fluorescent_probe_visualization(size, wavelength, threshold)` — Sonda fluorescente (anillo 6 átomos + nube π pulsante)
- `fret_pair_visualization(dx, dy, dz, ax, ay, az, R0, threshold)` — Par FRET (distancia calculada internamente)
- `protein_inhibitor_visualization(ix, iy, iz, affinity, threshold)` — Inhibidor
- `combinatorial_library_visualization(sx, sy, sz, points, threshold)` — Librería (nódulos periódicos)
- `protein_labeling_visualization(radius, sx, sy, sz, fx, fy, fz, threshold)` — Etiquetado
- `allosteric_modulator_visualization(ox, oy, oz, ax, ay, az, mx, my, mz, threshold)` — Modulación alostérica

## Notas
La biología química aplica herramientas químicas a sistemas biológicos. Las sondas moleculares permiten visualizar procesos celulares. QSAR predice actividad basándose en estructura. Las reglas de fármaco guían el diseño de moléculas bioactivas.

Corregido 2026-10-17: `union` es keyword (no puede nombrar variables — `let union` en Tanimoto); no hay listas/`for`/indexación: Tanimoto, QSAR, cobertura, diversidad y los 6 defs pasaron a firmas escalares o paramétricas (nódulos periódicos vía `cos(n·atan(z−sz, x−sx))`, cápsulas vía `segment_distance`). `ligand_efficiency` usaba `ic50_target` inexistente → LE = −ΔG/(N·RT) con ΔG en J/mol; `fluorescent_probe_efficiency` multiplicaba el brillo dos veces → ahora Φ·ε; ε de FRET reducido a 1e-12 (a escala nm el 1e-6 dominaba).
