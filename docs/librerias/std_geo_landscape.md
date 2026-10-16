# std_geo_landscape.hm

**Capa:** 7.4 — Ecología de Paisajes y Biomas
**Versión:** 0.2
**Fuente:** Fractalidad de paisajes, Modelos de percolación de bosques
**Dependencias:** std_math_core.hm, std_math_noise.hm
**Fecha:** 6 de junio de 2026

## Cómo
Implementa teoría de percolación en redes cuadradas 2D (umbral crítico `p_c ≈ 0.592746`) y distribuciones espaciales de biomas. Usa ruido de Voronoi y FBM para generar macroestructuras como bosques fragmentados y redes de ríos fractales.

## Por qué
Modelar la fragmentación de hábitats y la distribución de ecosistemas requiere herramientas de percolación y ruido fractal. Esta librería conecta la ecología matemática con la generación procedural de paisajes.

## Para qué
Generar mapas de biomas, simular conectividad ecológica, crear bosques y ríos procedurales para visualización o juegos, calcular índices de aridez.

## Funciones principales

| Función | Descripción |
|---------|-------------|
| `percolation_threshold()` | Retorna `p_c ≈ 0.592746` (umbral de percolación en red cuadrada 2D) |
| `mean_cluster_size(p_occ, p_c, gamma_exp)` | Tamaño medio del clúster cerca del umbral (divergencia tipo ley de potencias) |
| `aridity_index(precipitation, pet)` | Índice de aridez P/PET |

## SDFs definidas

| SDF | Descripción |
|-----|-------------|
| `fragmented_forest(sc_val, density_threshold, tree_height)` | Bosque fragmentado umbralizado con Voronoi 2D |
| `fractal_river_network(sc_val, depth, width)` | Red de ríos fractales usando FBM 2D invertido con perfil gaussiano de valle |

## Notas
- Librería ligera (56 líneas) enfocada en macroestructuras de paisaje.
- La percolación es útil para determinar conectividad de corredores ecológicos en mapas de biomas.
- Las SDFs están diseñadas para combinarse con `std_geo_terrain.hm` en paisajes completos.
