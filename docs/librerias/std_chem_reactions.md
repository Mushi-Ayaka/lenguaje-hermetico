# std_chem_reactions.hm

**Capa:** 3.3 — Cinética Química y Termodinámica de Reacciones
**Versión:** 0.2
**Fuente:** Arrhenius, Eyring, van't Hoff, Le Chatelier
**Dependencias:** std_math_core.hm, std_phys_sdf.hm
**Fecha:** 6 de junio de 2026

## Cómo
Implementa modelos cinéticos clásicos: ley de velocidad de reacción, ecuación de Arrhenius, teoría del estado de transición (Eyring), cinética de primer orden, semivida, equilibrio de van't Hoff, grado de disociación, pH de ácidos débiles, solubilidad de sales AB, perfiles de energía de reacción y SDFs de barrera de activación y difusión de reactivo.

## Por qué
Conecta la química estática (átomos, enlaces) con la dinámica temporal de las reacciones. Permite modelar cómo y qué tan rápido ocurren las transformaciones químicas.

## Para qué
- Calcular velocidades de reacción y constantes cinéticas
- Modelar equilibrio químico dependiente de temperatura
- Estimar pH de ácidos débiles y solubilidad
- Visualizar perfiles de energía de reacción y barreras de activación

## Funciones principales
| Función | Descripción |
|---|---|
| `reaction_rate(k, [A], m, [B], n)` | Ley de velocidad de reacción |
| `arrhenius(A, Ea, R, T)` | Constante de Arrhenius |
| `eyring_rate(T, ΔG‡, R)` | Teoría del estado de transición |
| `first_order_conc([A]₀, k, t)` | Concentración en reacción de primer orden |
| `half_life_first_order(k)` | Semivida de primer orden |
| `equilibrium_K_at_T(K₁, ΔH, R, T₁, T₂)` | Constante de equilibrio de van't Hoff |
| `acid_dissociation_degree(Ka, [HA])` | Grado de disociación de ácido débil |
| `weak_acid_pH(Ka, [HA])` | pH de ácido débil |
| `solubility_AB(Ksp)` | Solubilidad de sal AB |

## SDFs definidas
| SDF | Descripción |
|---|---|
| `activation_barrier(Ea, x_ts, width)` | Barrera de activación (montículo gaussiano) |
| `reactant_diffusion(conc₀, k, t, threshold)` | Concentración de reactivo como difusión esférica |

## Notas
Constantes BOLTZMANN y PLANCK importadas de std_phys_sdf. Usa EPSILON para estabilidad numérica.
