# std_bio_genetics.hm

**Capa:** 6.2 — Genética y Epigenética
**Versión:** 1.0
**Fuente:** "Genetics: A Conceptual Approach" (Pierce), "Epigenetics" (Allis et al.)
**Dependencias:** std_math_core.hm, std_math_algebra.hm, std_math_calculus.hm, std_math_probability.hm, std_bio_cell.hm, std_bridge_biochem.hm
**Fecha:** 6 de junio de 2026

## Cómo
Implementa modelos de genética de poblaciones (Hardy-Weinberg, F-statistics), herencia mendeliana (monohíbrido, dihíbrido, autosómico recesivo), epigenética (metilación de ADN semiconservativa, herencia epigenética, islas CpG), replicación (Meselson-Stahl, fidelidad, velocidad de horquilla) y expresión génica (regulación Hill, silenciamiento por metilación, epistasia). Las SDFs representanestructuras moleculares y patrones genómicos.

## Por qué
La genética explica la transmisión de caracteres entre generaciones; la epigenética añade la capa reguladora que modula la expresión sin cambiar la secuencia. Ambas son esenciales para modelar herencia, enfermedades y plasticidad fenotípica.

## Para qué
Simular frecuencias alélicas en poblaciones, calcular probabilidades de cruzamientos mendelianos, modelar dinámicas de metilación, visualizar la doble hélice de ADN y cromosomas en 3D, y estudiar redes de regulación génica.

## Funciones principales
- `hardy_weinberg_equilibrium(p)` — Frecuencia de heterocigotos (2pq)
- `allele_frequency_from_genotypes(fAA, fAa, faa)` — Frecuencia alélica desde genotipos
- `hardy_weinberg_deviation(H_obs, p, N)` — Estadístico F de desviación
- `monohybrid_cross_probability(g1, g2, dominant)` — Probabilidad de fenotipo dominante/recesivo
- `autosomal_recessive_carrier_prob(affected, total)` — Probabilidad de portador
- `dihybrid_independent_assortment(p1, p2)` — Segregación independiente
- `dna_methylation_dynamics(m, α, β)` — Dinámica de metilación post-replicación
- `epigenetic_inheritance(parent_m, fidelity, noise)` — Herencia epigenética
- `cpg_island_methylation(gc, threshold, steepness)` — Densidad de metilación CpG
- `semiconservative_replication(generations)` — Fracción de ADN parental
- `dna_replication_fidelity(length, error_rate)` — Probabilidad de copia correcta
- `replication_fork_speed(base_rate, efficiency, complexity)` — Velocidad de horquilla
- `transcriptional_regulation([A], Vmax, Kd, n)` — Regulación Hill
- `methylation_silencing(basal, methylation, sensitivity)` — Silenciamiento epigenético
- `epistatic_interaction(e1, e2, strength)` — Interacción epistática

## SDFs definidas
- `dna_double_helix(radius, pitch, spacing, threshold)` — Doble hélice de ADN
- `metaphase_chromosome(length, centromere, thickness, threshold)` — Cromosoma metafásico con centrómero
- `cpg_methylation_pattern(scale, density, threshold)` — Patrón de metilación CpG

## Notas
- Las constantes (AVOGADRO_GENETICS, BASE_PAIR_MASS, GENOME_SIZE_HUMAN, etc.) son valores típicos para visualización y simulación pedagógica.
- Los modelos mendelianos asumen dominancia completa y segregación independiente.
- Para organismos no humanos, ajustar constantes y tasas específicas.
- La SDF de cromosoma usa cápsulas vía `custom` (cilindros con extremos redondeados) y esfera para el centrómero.
- Corregido 2026-10-17: `cylinder` con extremos vectoriales no existe en el lenguaje (era error de sintaxis; reescrito con `custom`), `atan2` → `atan(y, x)`, y la matriz monohíbrida ahora usa doble umbral → Aa×Aa da 0.75 (antes daba 1.0) y aa×aa da 0 (antes 0.25).
