# std_geo_ocean.hm

**Capa:** 7.3 — Oceanografía y Dinámica de Fluidos
**Versión:** 0.2
**Fuente:** Modelo de Ekman, Navier-Stokes, Mareas Astronómicas
**Dependencias:** std_math_core.hm, std_geo_atmosphere.hm
**Fecha:** 6 de junio de 2026

## Cómo
Implementa la espiral de Ekman (profundidad y velocidad superficial en función del viento, viscosidad y Coriolis) y fuerzas de marea astronómicas simplificadas (magnitud de fuerza de marea y elevación de equilibrio armónico). Incluye SDFs para topografía de fondo oceánico y remolinos.

## Por qué
La dinámica oceánica es esencial para simular corrientes, mezcla vertical y mareas en entornos planetarios. Depende de `std_geo_atmosphere.hm` para el parámetro de Coriolis y la densidad del aire.

## Para qué
Modelar corrientes de Ekman, calcular alturas de marea, generar fosas abisales, plataformas continentales y remolinos oceánicos procedurales.

## Funciones principales

| Función | Descripción |
|---------|-------------|
| `ekman_depth(A_z_viscosity, coriolis_f)` | Profundidad de Ekman (influencia del viento) |
| `ekman_surface_velocity(wind_stress, water_density, A_z_viscosity, coriolis_f)` | Velocidad superficial de Ekman |
| `tidal_force_magnitude(G_const, M_body, R_planet, D_distance)` | Magnitud de fuerza de marea relativa `~ 2GMR/D³` |
| `equilibrium_tide_height(h_max, theta_angle)` | Elevación de marea de equilibrio estática simplificada |

## SDFs definidas

| SDF | Descripción |
|-----|-------------|
| `ocean_trench(trench_depth, trench_width, sc_val)` | Fosa abisal con plataforma y ruido FBM 2D |
| `ocean_eddy(radius, depth, swirl_strength)` | Remolino oceánico (eddy) con deformación sinusoidal |

## Notas
- Las mareas usan aproximación de equilibrio estático; no incluye efectos de cuenca ni resonancia.
- Depende de `std_geo_atmosphere.hm` para el parámetro de Coriolis y densidad del aire.
- Las SDFs de fondo oceánico pueden combinarse con `std_geo_terrain.hm` para terreno sumergido.
