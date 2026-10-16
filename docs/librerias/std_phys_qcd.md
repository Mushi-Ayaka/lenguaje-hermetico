# std_phys_qcd.hm

**Capa:** 2.6 — Cromodinámica Cuántica (QCD)
**Versión:** 0.2
**Fuente:** Gell-Mann, Gross, Wilczek, Politzer
**Dependencias:** std_math_core.hm, std_phys_sdf.hm
**Fecha:** 6 de junio de 2026

## Cómo
Implementa el potencial de Cornell (confinamiento + Coulomb), la constante de acoplamiento running (libertad asintótica) y el modelo de la bolsa de MIT (energía y radio de equilibrio del hadrón).

## Por qué
QCD describe la interacción fuerte que mantiene unidos quarks y gluones. Esta librería traduce sus modelos fenomenológicos clave a funciones y SDFs del Lenguaje Hermético.

## Para qué
- Visualizar tubos de flujo de color entre quarks
- Modelar bolsas de confinamiento (hadrones) como SDFs
- Simular el potencial quark-antiquark y la libertad asintótica

## Funciones principales

| Función | Descripción |
|---|---|
| `cornell_potential(r, α_s, σ)` | Potencial quark-antiquark: −4α_s/3r + σ·r |
| `running_coupling(Q², β₀)` | Acoplamiento running: 1/(β₀·ln(Q²/Λ²)) |
| `mit_bag_energy(R, C, B)` | Energía de la bolsa de MIT: C/R + ⁴⁄₃πR³B |
| `mit_bag_radius(C, B)` | Radio de equilibrio: (C/4πB)^(¹/₄) |

## SDFs definidas

| SDF | Descripción |
|---|---|
| `color_flux_tube(q_dist, tube_radius)` | Tubo de flujo de color entre dos quarks en eje X |
| `hadron_bag(C, B)` | Bolsa de confinamiento hadrónica (esfera del radio de MIT) |

## Notas
- `LAMBDA_QCD` = 0.2 (escala QCD ≈ 200 MeV en unidades naturales).
- El modelo de Cornell combina un término Coulombiano a corta distancia con confinamiento lineal a larga distancia.
- La bolsa de MIT es un modelo fenomenológico simple de hadrones.
