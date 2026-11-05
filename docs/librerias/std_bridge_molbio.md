# std_bridge_molbio.hm

**Capa:** Intermediario: Química-Biología (5.3 — Biología Molecular)
**Versión:** 1.0
**Fuente:** Watson & Crick "DNA Structure", Alberts "Molecular Biology of the Cell"
**Dependencias:** std_math_core.hm, std_bridge_atomic.hm
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

- Corregido 2026-10-17: `log10(x)` no existe en el lenguaje → `log(x)/ln(10)` (log unaria = log natural); 10 defs reescritos sin `let`/`fn` anidadas; `for i in range(...)` del replisoma → 4 polimerasas desplegadas en cadena; `sphere([vec],r)`, `cylinder([vec],[vec],r)`, `box([vec],[vec])` y `torus([vec],[vec],R,r)` no son válidos (las primitivas solo toman escalares centrados en origen) → cajas/cápsulas/esferas/anillos posicionados vía `custom` + `segment_distance` de std_bridge_atomic; uniones de 3-7 hijos → anidadas estrictamente binarias; `custom(mrna_base + exon_pattern)` (suma nodo SDF + expr) → cápsula con modulación radial real; helice doble: superficies implícitas helicoidales `sqrt((x−r·cosθ)²+(z−r·sinθ)²)−w` con θ=2π·(y·length·2)/pitch y columna central de pares de bases con abalorios cada 3.4Å; `fork_angle` (normalizado a 45°), `promoter_length` (crestas de la DNA) y `polymerase_count` (radio de polimerasas) ahora parametrizan la geometría.
- Verificado 2026-10-17: compila (exit 0); probe 20 nodos / 51 customs exactos; checks: Tm(50%GC, 0.05M, 100pb)=77.15°C y 1M=98.75°C, hélice on-surface −0.04 en dos puntos de parametrización (y=0 y θ=π/2).
