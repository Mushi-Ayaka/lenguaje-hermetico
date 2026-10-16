# std_bridge_chembio.hm

**Capa:** Intermediario Química-Biología (5.4 — Biología Química)
**Versión:** 1.0
**Fuente:** "Chemical Biology" by Schreiber, Kapoor & Wess
**Dependencias:** std_math_core.hm, std_math_algebra.hm, std_chem_organic.hm, std_bridge_biochem.hm, std_bridge_molbio.hm
**Fecha:** 6 de junio de 2026

## Cómo
Proporciona funciones para sondas moleculares (eficiencia fluorescente, FRET, fotoblanqueo, Stokes shift), inhibidores (IC50, selectividad, ventana terapéutica, ligand efficiency, LLE), química combinatoria (tamaño de librería, diversidad molecular, Tanimoto, cobertura de espacio químico), actividad biológica (QSAR, regla de Lipinski, fracción sp3, drug-likeness), y conjugación (eficiencia, F/P ratio, grado de sustitución, retención de actividad).

## Por qué
Aplicar herramientas químicas sintéticas y analíticas a sistemas biológicos para diseñar sondas, inhibidores y moléculas bioactivas.

## Para qué
Usada para modelar herramientas de biología química: fluoróforos, FRET, inhibidores enzimáticos, librerías combinatorias, predicción QSAR, diseño de fármacos, y bioconjugación.

## Funciones principales
- `fluorescent_probe_efficiency(quantum_yield, extinction, brightness)` — Brillo = Φ·ε
- `fret_efficiency(donor_emission, acceptor_absorption, distance, R0)` — Eficiencia FRET
- `photobleaching_rate(intensity, photosensitizer, oxygen)` — Tasa de fotoblanqueo
- `competitive_inhibition_ic50(ki, S, km)` — IC50 = Ki·(1 + [S]/Km)
- `selectivity_index(ic50_target, ic50_offtarget)` — Índice de selectividad
- `therapeutic_window(toxic_dose, effective_dose)` — Ventana terapéutica
- `ligand_efficiency(binding_energy, heavy_atom_count)` — LE
- `lipophilic_ligand_efficiency(pic50, logP)` — LLE
- `combinatorial_library_size(blocks, positions)` — Tamaño de librería
- `tanimoto_similarity(fp1, fp2)` — Similitud de Tanimoto
- `qsar_prediction(descriptors, coefficients, intercept)` — QSAR lineal
- `lipinski_rule_of_five(MW, logP, HBD, HBA)` — Regla de los 5
- `druglikeness_score(MW, logP, HBD, HBA, rot, aromatic)` — Score de fármaco
- `conjugation_efficiency(reactant, product, time)` — Eficiencia de conjugación

## SDFs definidas
- `fluorescent_probe_visualization(size, wavelength, threshold)` — Sonda fluorescente
- `fret_pair_visualization(donor, acceptor, distance, R0, threshold)` — Par FRET
- `protein_inhibitor_visualization(site, structure, affinity, threshold)` — Inhibidor
- `combinatorial_library_visualization(points, scaffold, blocks, threshold)` — Librería
- `protein_labeling_visualization(surface, sites, fluorophores, threshold)` — Etiquetado
- `allosteric_modulator_visualization(ortho, allo, modulator, threshold)` — Modulación alostérica

## Notas
La biología química aplica herramientas químicas a sistemas biológicos. Las sondas moleculares permiten visualizar procesos celulares. QSAR predice actividad basándose en estructura. Las reglas de fármaco guían el diseño de moléculas bioactivas.
