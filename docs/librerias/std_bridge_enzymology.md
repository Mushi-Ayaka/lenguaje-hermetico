# std_bridge_enzymology.hm

**Capa:** Intermediario Química-Biología (5.6 — Enzimología)
**Versión:** 1.0
**Fuente:** "Enzymes" by Bugg
**Dependencias:** std_math_core.hm, std_math_algebra.hm, std_bridge_biochem.hm, std_bridge_kinetics.hm, std_bridge_chembio.hm
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
- `multi_substrate_competition(Vmax, A, B, KmA, KmB, mechanism)` — Dos sustratos
- `two_substrate_kinetics(S1, S2, mechanism, constants)` — Cinética bi-bi
- `competitive_inhibition_velocity(Vmax, S, I, Km, Ki)` — Inhibición competitiva
- `noncompetitive_inhibition_velocity(Vmax, S, I, Km, Ki)` — No competitiva
- `uncompetitive_inhibition_velocity(Vmax, S, I, Km, Ki)` — Acompetitiva
- `mixed_inhibition_velocity(Vmax, S, I, Km, alpha, beta)` — Mixta
- `feedback_inhibition(activity, product, Ki, cooperativity)` — Retroalimentación
- `catalytic_rate_acceleration(kcat, k_uncat)` — Aceleración catalítica
- `general_acid_base_catalysis(ph, pka1, pka2)` — Catálisis ácido-base
- `covalent_catalysis_rate(nucleophile, electrophile, leaving_group)` — Catálisis covalente
- `strain_catalysis(distortion, complementarity)` — Catálisis de tensión
- `mwc_allosteric_model(S, r, t, L0, c)` — Modelo Monod-Wyman-Changeux
- `knf_allosteric_model(S, K, cooperativity)` — Modelo Koshland-Némethy-Filmer
- `hill_coefficient(concentrations, activities)` — Coeficiente de Hill
- `catalytic_promiscuity(main, secondary_activities)` — Promiscuidad catalítica
- `affinity_evolution(ancestral, descendant, mutations)` — Evolución de afinidad

## SDFs definidas
- `enzyme_active_site_visualization(residues, cofactor, substrate, threshold)` — Sitio activo
- `acid_base_catalysis_visualization(acid, base, substrate, proton, threshold)` — Catálisis ácido-base
- `competitive_inhibition_visualization(site, substrate, inhibitor, strength, threshold)` — Inhibición
- `allosteric_regulation_visualization(active, allosteric, effector, path, threshold)` — Alosterismo
- `covalent_catalysis_visualization(enzyme, substrate, intermediate, threshold)` — Catálisis covalente
- `catalytic_promiscuity_visualization(site, substrates, activities, threshold)` — Promiscuidad

## Notas
Las enzimas son catalizadores biológicos que aceleran reacciones millones de veces. La inhibición puede ser competitiva, no competitiva, acompetitiva o mixta. Los mecanismos catalíticos incluyen ácido-base, covalente y de tensión. La regulación alostérica permite control fino. Las enzimas evolucionan adquiriendo nuevas funciones (promiscuidad catalítica).
