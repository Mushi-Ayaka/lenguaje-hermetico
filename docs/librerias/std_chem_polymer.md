# std_chem_polymer.hm

**Capa:** 3.6 — Química de Polímeros y Materiales Blandos
**Versión:** 0.2
**Fuente:** Flory, de Gennes, teoría de cadenas estadísticas
**Dependencias:** std_math_core.hm, std_math_probability.hm, std_math_noise.hm
**Fecha:** 6 de junio de 2026

## Cómo
Proporciona estadística de cadenas poliméricas (radio de giro, distancia extremo-extremo, longitud de persistencia, distribución gaussiana), termodinámica de Flory-Huggins (energía de mezcla, parámetro crítico), mecánica de geles y elastómeros (módulo de elasticidad, presión osmótica), y SDFs de morfologías poliméricas (cadena sinuosa con FBM, red entrecruzada, glóbulo colapsado, micela, vesícula bicapa).

## Por qué
Los polímeros y materiales blandos (geles, elastómeros, micelas) son ubicuos en química de materiales y biología. Se requiere modelar su comportamiento estadístico y termodinámico.

## Para qué
- Calcular propiedades conformacionales de cadenas poliméricas
- Modelar termodinámica de mezclas polímero-solvente (Flory-Huggins)
- Estimar propiedades mecánicas de geles y elastómeros
- Visualizar morfologías: cadenas, redes, glóbulos, micelas, vesículas

## Funciones principales
| Función | Descripción |
|---|---|
| `radius_of_gyration(l, N)` | Radio de giro (Rg = l√(N/6)) |
| `end_to_end_rms(l, N)` | Distancia RMS extremo-extremo |
| `persistence_length(κ, k_B, T)` | Longitud de persistencia |
| `polymer_end_to_end_prob(R, l, N)` | Distribución gaussiana de extremo-extremo |
| `flory_huggins_dG(φ₁, N, χ, kT)` | Energía de mezcla de Flory-Huggins |
| `flory_chi_critical(N)` | Parámetro crítico de Flory |
| `rubber_modulus(ρ, k_B, T)` | Módulo de elasticidad de red |
| `osmotic_pressure(c, N, B₂, k_B, T)` | Presión osmótica (Flory) |

## SDFs definidas
| SDF | Descripción |
|---|---|
| `polymer_chain(len, tube_r, noise_scale, noise_amp)` | Cadena polimérica sinuosa con FBM |
| `polymer_network(cell_size, strand_r)` | Red polimérica entrecruzada (foam) |
| `polymer_globule(r, d_scale)` | Glóbulo colapsado con FBM |
| `micelle(r_core, shell_thick)` | Micela esférica (capa + núcleo) |
| `vesicle(r, membrane_thick, gap)` | Vesícula bicapa (doble membrana) |

## Notas
Usa FBM (fractional Brownian motion) de std_math_noise para cadenas sinuosas. Las SDFs de micela y vesícula son de capa delgada (abs() para cascarones esféricos).
