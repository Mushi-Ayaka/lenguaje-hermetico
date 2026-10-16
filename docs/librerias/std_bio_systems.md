# std_bio_systems.hm

**Capa:** 6.8 — Biología de Sistemas
**Versión:** 0.2
**Fuente:** Análisis de Control Metabólico (MCA), Redes de Regulación
**Dependencias:** std_math_core.hm
**Fecha:** 6 de junio de 2026

## Cómo
Implementa coeficientes de Análisis de Control Metabólico (coeficiente de control de flujo, coeficiente de elasticidad), modelos de regulación génica (auto-regulación negativa, feed-forward loop coherente tipo 1 con lógica AND), y una SDF de topología de red libre de escala para visualización de nodos y conexiones.

## Por qué
La biología de sistemas estudia el comportamiento emergente de redes biológicas (metabólicas, genéticas, de señalización). No basta con conocer los componentes; hay que entender cómo sus interacciones producen propiedades sistémicas.

## Para qué
Calcular coeficientes de control metabólico (qué enzimas limitan el flujo), modelar motivos de redes de regulación genética (auto-regulación, feed-forward loops), y visualizar topologías de red libre de escala en 3D.

## Funciones principales
- `flux_control_coefficient(J, dJ, v, dv)` — Coeficiente de control de flujo (MCA)
- `elasticity_coefficient(v, dv, S, dS)` — Coeficiente de elasticidad (MCA)
- `negative_autoregulation(X, β, α, K, n)` — Dinámica de auto-represión
- `ffl_coherent_and(X, Y, Z, β, α, Kx, Ky, n)` — Feed-forward loop coherente AND

## SDFs definidas
- `scale_free_network_node(r_hub, r_periph, n_spokes)` — Nodo de red libre de escala con spokes radiales

## Notas
- MCA requiere perturbaciones pequeñas alrededor del estado estacionario para ser válido.
- El feed-forward loop coherente tipo 1 (C1-FFL) actúa como detector de persistencia en señales.
- La SDF usa `sat(cos(n * atan(z, x)))` para generar spokes radiales alrededor de un hub central.
- La biblioteca es pequeña pero conceptualmente densa; se espera expansión futura con más motivos de red.
