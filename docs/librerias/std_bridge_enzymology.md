# std_bridge_enzymology.hm

**Capa:** Intermediario Química-Biología (5.6 — Enzimología)
**Versión:** 1.0
**Fuente:** "Enzymes" by Bugg
**Dependencias:** std_math_core.hm, std_bridge_atomic.hm
**Fecha:** 6 de junio de 2026

## Cómo
Define constantes fundamentales (Boltzmann, Avogadro, gas, Planck) y funciones para cinética enzimática (Michaelis-Menten, kcat/Km, Briggs-Haldane, multi-sustrato ordenado/aleatorio/ping-pong, bi-bi), inhibición (competitiva, no competitiva, acompetitiva, mixta, retroalimentación, reactivación), catálisis (aceleración catalítica, eficiencia, contribución de energía libre, ácido-base general, covalente, tensión/straIN), sitio activo (composición, accesibilidad, complementariedad al estado de transición, preorganización, desolvatación), regulación (MWC, KNF, coeficiente de Hill, regulación covalente, compartimentalización, sensibilidad), y evolución enzimática (diversidad de especificidad, robustez, promiscuidad catalítica, evolución de afinidad, presión selectiva).

## Por qué
Modelar cuantitativamente la catálisis enzimática, los mecanismos de inhibición, la regulación alostérica y la evolución de enzimas.

## Para qué
Usada para representar cinéticas enzimáticas, mecanismos catalíticos, diseño de inhibidores, regulación alostérica y evolución de funciones enzimáticas en el Lenguaje Hermético.

## Funciones principales
- `michaelis_menten_kinetics(S, Vmax, Km)` — Velocidad Michaelis-Menten
- `specificity_constant(kcat, Km)` — kcat/Km
- `turnover_number(Vmax, enzyme_conc)` — kcat
- `steady_state_velocity(k1, k_neg1, k2, S, E)` — Briggs-Haldane
- `multi_substrate_competition(Vmax, A, B, KmA, KmB, mechanism)` — Dos sustratos (mechanism: 0=ordered, 1=random, 2=ping-pong)
- `two_substrate_kinetics(S1, S2, mechanism, K1, K2, Vmax)` — Cinética bi-bi (mechanism numérico 0/1/2)
- `competitive_inhibition_velocity(Vmax, S, I, Km, Ki)` — Inhibición competitiva
- `noncompetitive_inhibition_velocity(Vmax, S, I, Km, Ki)` — No competitiva
- `uncompetitive_inhibition_velocity(Vmax, S, I, Km, Ki)` — Acompetitiva
- `mixed_inhibition_velocity(Vmax, S, I, Km, alpha_mixed, beta_mixed)` — Mixta
- `feedback_inhibition(activity, product, Ki, cooperativity)` — Retroalimentación
- `catalytic_rate_acceleration(kcat, k_uncat)` — Aceleración catalítica
- `general_acid_base_catalysis(ph, pka1, pka2)` — Catálisis ácido-base
- `covalent_catalysis_rate(nucleophile, electrophile, leaving_group)` — Catálisis covalente
- `strain_catalysis(distortion, complementarity)` — Catálisis de tensión
- `mwc_allosteric_model(S, r, t, L0, c)` — Modelo Monod-Wyman-Changeux
- `knf_allosteric_model(S, K, cooperativity)` — Modelo Koshland-Némethy-Filmer
- `hill_coefficient(S1, S2, v1, v2)` — Coeficiente de Hill (2 puntos)
- `catalytic_promiscuity(main_activity, secondary1, secondary2)` — Promiscuidad catalítica
- `substrate_specificity_diversity(sp1, sp2)` — Índice tipo Shannon (2 constantes)
- `affinity_evolution(ancestral, descendant, mutations)` — Evolución de afinidad

## SDFs definidas
- `enzyme_active_site_visualization(n_residues, sx, sy, sz, cx, cy, cz, threshold)` — Sitio activo (anillo de n residuos en xy)
- `acid_base_catalysis_visualization(ax..az, bx..bz, sx..sz, proton_transfer, threshold)` — Catálisis ácido-base (protón según fracción)
- `competitive_inhibition_visualization(asx..asz, sx..sz, ix..iz, strength, threshold)` — Inhibición
- `allosteric_regulation_visualization(asx..asz, lsx..lsz, ex..ez, threshold)` — Alosterismo
- `covalent_catalysis_visualization(ex..ez, sx..sz, ix..iz, threshold)` — Catálisis covalente
- `catalytic_promiscuity_visualization(asx..asz, s1x..s1z, s2x..s2z, a1, a2, threshold)` — Promiscuidad

## Notas
Las enzimas son catalizadores biológicos que aceleran reacciones millones de veces. La inhibición puede ser competitiva, no competitiva, acompetitiva o mixta. Los mecanismos catalíticos incluyen ácido-base, covalente y de tensión. La regulación alostérica permite control fino. Las enzimas evolucionan adquiriendo nuevas funciones (promiscuidad catalítica).

Corregido 2026-10-17: `||` no existe en el lexer (eliminado junto con las comparaciones de strings); `alpha` es keyword → `alpha_mixed`; los mecanismos ordered/random/ping-pong usan selector numérico (0/1/2) con `step`; arrays/`for`/`max_array`/`min_array`/`contains` → versiones escalares (Hill a 2 puntos, Shannon a 2 especies, promiscuidad a 2 actividades, composición por presencias 0/1); `catalytic_efficiency` referenciaba `enzyme_concentration` no declarada → ahora parámetro; los 6 defs sin listas: posiciones escalares, anillo de residuos como nódulos periódicos `cos(n·atan(y,x))` y enlaces con `segment_distance`; la opcionalidad (cofactor/`random()`) se resolvió pasando la posición escalar (radio 0 = ausente).
