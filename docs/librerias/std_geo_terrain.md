# std_geo_terrain.hm

**Capa:** 4.1 — Geología, Erosión y Modelado de Terreno
**Versión:** 0.2
**Fuente:** Procedural terrain, hydraulic erosion, sediment transport
**Dependencias:** std_math_core.hm, std_math_noise.hm
**Fecha:** 6 de junio de 2026

## Cómo
Genera terreno procedural mediante FBM clásico (terrain_height), FBM con crestas (ridged_fbm) y FBM suavizado (billowy_fbm). Modela erosión hidráulica simplificada con cálculo de pendiente, velocidad de flujo, capacidad de sedimento y tasa de erosión neta. Incluye capas geológicas sedimentarias con buzamiento, pliegues (anticlinal/sinclinal) y perfiles volcánicos.

## Por qué
Es la librería fundamental para modelado de relieve terrestre y planetario. Conecta la geología física con la generación procedural para crear terrenos realistas y estratigráficamente coherentes.

## Para qué
Renderizar montañas, valles, crestas, volcanes, calderas, cuevas kársticas, estratos geológicos y simular erosión hídrica y meteorización térmica.

## Funciones principales

| Función | Descripción |
|---------|-------------|
| `terrain_height(x_km, z_km, sc_val, H_max)` | Altura de terreno FBM con 6 octavas |
| `ridged_fbm(x_val, z_val, sc_val)` | FBM con crestas para montañas agudas |
| `billowy_fbm(x_val, z_val, sc_val)` | FBM suavizado para colinas redondeadas |
| `slope_x(h_prev, h_next, delta)` | Gradiente de pendiente en X |
| `slope_z(h_prev, h_next, delta)` | Gradiente de pendiente en Z |
| `flow_speed(slope_x_val, slope_z_val)` | Velocidad de flujo de agua |
| `sediment_capacity(k_c, speed)` | Capacidad de sedimento: `k_c * speed` |
| `erosion_rate(K_e, Q_actual, Q_capacity)` | Tasa de erosión/deposición neta |
| `thermal_weathering(T_amplitude, rock_hardness)` | Meteorización térmica |
| `sediment_layer(depth, thickness, dip_angle, dip_dir)` | Capa sedimentaria con buzamiento |
| `anticline(x_val, z_val, amplitude, wavelength, axis_x)` | Pliegue anticlinal (positivo) |
| `syncline(x_val, z_val, amplitude, wavelength, axis_x)` | Pliegue sinclinal (negativo) |
| `volcano_profile(r_base, h_max, r_crater)` | Perfil de volcán compuesto cónico |
| `caldera_depth(r_caldera, h_caldera)` | Depresión caldérica en la cima |

## SDFs definidas

| SDF | Descripción |
|-----|-------------|
| `mountain_terrain(terrain_scale, max_height)` | Isosuperficie de terreno FBM |
| `ridged_terrain(terrain_scale, max_height)` | Isosuperficie de crestas montañosas |
| `composite_volcano(r_base, h_max, r_crater)` | Volcán compuesto con caldera |
| `karst_cave(cave_scale, iso_threshold)` | Cueva kárstica por FBM 3D umbralizado |
| `geological_strata(strata_thickness, wave_amplitude, wave_frequency)` | Estratos geológicos ondulados |

## Notas
- Librería más extensa de las geociencias (118 líneas) por la amplitud de fenómenos geológicos que cubre.
- La erosión hidráulica es una versión simplificada (1D de flujo); no simula transporte 2D completo.
- Los pliegues (anticlinal/sinclinal) usan una envolvente gaussiana para localizar la deformación.
- Las SDFs de terreno pueden combinarse con `std_geo_landscape.hm` para ecosistemas completos.
