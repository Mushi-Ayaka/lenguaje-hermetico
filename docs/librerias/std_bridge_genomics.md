# std_bridge_genomics.hm

**Capa:** Intermediario Química-Biología (5.7 — Genómica Química y Metabolómica)
**Versión:** 1.0
**Fuente:** "Chemical Genomics" & "Metabolomics: From Fundamentals to Clinical Applications"
**Dependencias:** std_math_core.hm, std_bridge_atomic.hm
**Fecha:** 6 de junio de 2026

## Cómo
Define constantes genómicas (tamaño del genoma humano, GC content, metaboloma, número de genes) y funciones para genómica química (densidad GC, unión fármaco-ADN, especificidad de secuencia, activación química de TF, inhibición epigenética, modificación de cromatina), quimiogenómica (pantalla de fenotipo, índice de selectividad genómica, perfil de expresión, mapeo de interacciones, validación de dianas), metabolómica (ratios metabólicos, fold change, estrés metabólico, energía celular, perfil normalizado, similitud de patrones, enriquecimiento de rutas, biomarcadores), y metabolómica fluxómica (flujo metabólico, balance de flujos, Arrhenius, control de flujo, análisis de estabilidad).

## Por qué
Integrar química y genómica para descubrimiento de fármacos y análisis sistemático de metabolitos como biomarcadores de estado biológico.

## Para qué
Usada para modelar interacciones molecular-genoma, pantallas quimiogenómicas, perfiles metabólicos, flujos metabólicos y diseño de herramientas CRISPR-químicas.

## Funciones principales
- `gc_content_density(gc_fraction, sequence_position, scale)` — Densidad GC genómica (`position` es keyword, por eso `sequence_position`)
- `dna_drug_binding_score(intercalation, groove, charge)` — Score de unión a ADN
- `sequence_specificity(target, off_target, penalty)` — Especificidad de secuencia
- `chemical_tf_activation(ligand, affinity, cooperativity, basal)` — Activación Hill-like de TF
- `chemical_epigenetic_inhibition(methylation, histone, enzyme)` — Inhibición epigenética
- `chemical_phenotype_screen_score(hit_rate, effect, reproducibility)` — Score de pantalla
- `chemical_expression_profile(up, down, threshold)` — Perfil de expresión inducida
- `metabolic_ratio(A, B)` — Ratio entre metabolitos
- `metabolic_fold_change(treated, control)` — Fold change log2 (ln x / ln 2)
- `metabolic_stress_index(lactate, pyruvate, NADH_NAD)` — Índice de estrés
- `cellular_energy_ratio(ATP, ADP, AMP)` — Ratio energético
- `metabolic_profile_normalized(m1, m2, ref1, ref2)` — Perfil normalizado (2 metabolitos)
- `metabolic_pattern_similarity(a1, a2, a3, b1, b2, b3)` — Similitud coseno (3 componentes)
- `metabolic_pathway_enrichment(pathway, background, size)` — Enriquecimiento de ruta
- `metabolic_biomarker_score(specificity, sensitivity, FC, reproducibility)` — Score de biomarcador
- `metabolic_flux(S, enzyme, kcat, km)` — Flujo Michaelis-Menten
- `metabolic_flux_control_coefficient(flux_change, enzyme_change)` — Coeficiente de control
- `metabolic_stability_analysis(eigenvalue1, eigenvalue2)` — Estabilidad (producto de 2 eigenvalores)

## SDFs definidas
- `dna_sequence_visualization(n_bases, base_rise, threshold)` — Secuencia de ADN (doble hélice paramétrica + peldaños periódicos)
- `drug_dna_interaction_visualization(dx..dz, px..pz, strength, threshold)` — Fármaco-ADN (cápsula real)
- `metabolic_network_visualization(n_nodes, mean_flux, threshold)` — Red metabólica (anillo de nodos + disco)
- `metabolic_profile_visualization(n_series, n_points, time_span, threshold)` — Perfil metabólico 3D + ejes
- `crispr_chemical_system_visualization(tx..tz, gx..gz, cx..cz, ex..ez, threshold)` — CRISPR-Chem (4 cápsulas de interacción)
- `chemical_phenotype_screen_visualization(n_wells, mean_hit, threshold)` — Pantalla (anillo + burbujas + leyenda 5 puntos)

## Notas
La genómica química integra química y genómica para descubrimiento de fármacos. La metabolómica estudia perfiles de metabolitos como biomarcadores. El análisis de flujos metabólicos modela dinámicas de redes bioquímicas. Los sistemas CRISPR-Chem combinan edición genómica con modulación química.

Corregido 2026-10-17: `position` es keyword (campo de nodo) → `sequence_position`; `log(2, x)` (2 args) no existe → log2 = ln x/ln 2; `<` léxico eliminado; arrays/`for` → normalización a 2 metabolitos, similitud coseno a 3 componentes, estabilidad = producto de 2 eigenvalores; los 6 defs pasan a parámetros escalares con hélices/anillos paramétricos (`cos(n·atan(z,x))`, `sin(pi·y/paso)`) y cápsulas `segment_distance`; el "sitio de interacción" invertido (−r²+0.04, no era superficie válida) se reemplazó por cápsula fármaco-DNA.
