# std_bridge_molbio.hm

**Capa:** Intermediario: Química-Biología (5.3 — Biología Molecular)
**Versión:** 1.0
**Fuente:** Watson & Crick "DNA Structure", Alberts "Molecular Biology of the Cell"
**Dependencias:** std_math_core.hm, std_math_algebra.hm, std_math_calculus.hm, std_bridge_biochem.hm, std_chem_polymer.hm
**Fecha:** 6 de junio de 2026

## Cómo
Define constantes de biología molecular (distancias de ADN, tamaños de polimerasa y ribosoma, tasas de elongación) y funciones para estructura del ADN (coordenadas helicoidales, stacking, twist, roll/tilt, estabilidad GC/AT, temperatura de melting), replicación (velocidad, fidelidad, horquilla, fragmentos de Okazaki, remoción de cebadores), transcripción (unión a promotor, iniciación, elongación, terminación, procesamiento de RNA, vida media del mRNA), traducción (unión ribosoma, iniciación, elongación, plegamiento cotraduccional, terminación, modificaciones postraduccionales), regulación génica (factores de transcripción, activación, represión, silenciamiento epigenético, remodelación de cromatina, feedback loops), y reparación del ADN (BER, NER, MMR, NHEJ vs HR, DDR).

## Por qué
Modelar los procesos fundamentales del dogma central de la biología molecular (ADN → RNA → proteína) y sus mecanismos de regulación y reparación.

## Para qué
Usada para representar y simular replicación, transcripción, traducción, regulación génica, reparación del ADN y señalización celular en el Lenguaje Hermético.

## Funciones principales
- `dna_helix_coordinates(t, radius, pitch)` — Coordenadas de doble hélice B-DNA
- `dna_base_pair_spacing(context)` — Distancia entre pares de bases
- `dna_twist_angle(sequence)` — Ángulo de giro (~36°)
- `dna_base_pair_stability(is_gc_pair)` — 3 puentes H (GC) vs 2 (AT)
- `dna_melting_temperature(gc, salt, length)` — Tm del ADN
- `dna_replication_rate(processivity, nucleotides, ATP)` — Velocidad de replicación
- `dna_replication_fidelity(proofreading, mismatch_repair)` — Tasa de error
- `transcription_initiation_rate(polymerase_bound, factors)` — Iniciación
- `transcription_elongation_rate(processivity, NTPs)` — Elongación
- `translation_elongation_rate(processivity, tRNA, GTP)` — Elongación traduccional
- `cotranslational_folding(chaperones, rate)` — Plegamiento durante traducción
- `transcription_factor_binding(affinity, concentration, cooperativity)` — Unión de TF
- `dna_methylation_silencing(cpg, hdac)` — Silenciamiento epigenético
- `regulatory_feedback_loop(activator, repressor, delay)` — Feedback
- `base_excision_repair(glycosylase, AP, polB)` — BER
- `nucleotide_excision_repair(XPC, TFIIH, XPA, XPG)` — NER
- `mismatch_repair(MSH, MLH, PMS, exonuclease)` — MMR
- `double_strand_break_repair(Ku, PKcs, Lig4, Rad51)` — NHEJ + HR

## SDFs definidas
- `dna_double_helix_detailed(radius, pitch, length, twist)` — Doble hélice B-DNA
- `rna_polymerase_promoter_complex(size, length, bend)` — Complejo de transcripción
- `ribosome_translation_complex(size, mrna_length, polypeptide)` — Ribosoma traduciendo
- `dna_replication_fork(parent_len, daughter_len, angle)` — Horquilla de replicación
- `condensed_chromosome(length, condensation, chromatids)` — Cromosoma metafásico
- `spliceosome_complex(mrna_length, pattern)` — Complejo de splicing
- `lac_operon_system(lactose, glucose)` — Operón Lac
- `replisome_machine(fork, polymerases, helicase)` — Replisoma
- `mapk_signaling_cascade(signal, phosphorylation)` — Cascada MAPK
- `proteasome_degradation_machine(activity, substrate, ubiquitin)` — Proteasoma

## Notas
Las estructuras moleculares son representaciones geométricas simplificadas. Los parámetros cinéticos y termodinámicos están escalados para visualización. Los sistemas de regulación son abstracciones de procesos reales complejos.
