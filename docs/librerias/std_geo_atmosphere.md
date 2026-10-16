# std_geo_atmosphere.hm

**Capa:** 4.2 — Ciencias Atmosféricas y Oceanografía
**Versión:** 0.2
**Fuente:** ICAO standard atmosphere, Navier-Stokes atmosférico
**Dependencias:** std_math_core.hm, std_math_calculus.hm, std_phys_sdf.hm, std_math_noise.hm
**Fecha:** 6 de junio de 2026

## Cómo
Define constantes atmosféricas estándar (ICAO) y funciones para modelar perfiles de temperatura, presión, densidad y velocidad del sonido en la troposfera. Incluye dinámica atmosférica (parámetro de Coriolis, viento geostrófico, vorticidad) y procesos de nubosidad (presión de vapor saturante, humedad relativa, nivel de condensación). También aborda oceanografía básica (velocidad de onda somera, ola de Stokes).

## Por qué
Proporciona un modelo físico reducido pero funcional de la atmósfera y el océano superficial para simular condiciones meteorológicas y oceanográficas en entornos procedurales o de visualización científica.

## Para qué
Generar campos de temperatura, presión y viento; simular nubes, ciclones y termoclinas oceánicas; servir como base para renderizado atmosférico y animación de fluidos geofísicos.

## Funciones principales

| Función | Descripción |
|---------|-------------|
| `std_temp(h_alt)` | Temperatura estándar ICAO a altura `h_alt` (m) en la troposfera |
| `std_pressure(h_alt)` | Presión barométrica estándar a partir de la temperatura |
| `air_density(h_alt)` | Densidad del aire vía ley de gases ideales |
| `speed_of_sound(h_alt)` | Velocidad del sonido en función de la temperatura |
| `coriolis_param(latitude_rad)` | Parámetro de Coriolis `f = 2Ω sin(φ)` |
| `geostrophic_wind(dP_dy, rho, f_coriolis)` | Viento geostrófico a partir del gradiente de presión |
| `rossby_deformation(N_brunt, h_atm, f_coriolis)` | Escala de Rossby (deformación) |
| `vorticity(dv_dx, du_dy)` | Vorticidad relativa 2D |
| `saturation_pressure(T_kelvin)` | Presión de vapor saturante (fórmula de Tetens) |
| `relative_humidity(vapor_pressure, T_kelvin)` | Humedad relativa |
| `lifting_condensation_level(T_celsius, Td_celsius)` | Nivel de condensación por elevación (LCL) |
| `shallow_wave_speed(depth)` | Velocidad de onda superficial en agua somera |
| `stokes_wave(amp, k, omega, x_val, t_val)` | Perfil de ola de Stokes (2º orden) |

## SDFs definidas

| SDF | Descripción |
|-----|-------------|
| `atmosphere_temperature(T_surface, T_iso)` | Campo escalar de temperatura atmosférica |
| `cumulus_cloud(cloud_scale, cloud_base, cloud_top, density_threshold)` | Nube cúmulo generada con FBM 3D y enmascaramiento vertical |
| `ocean_stokes_wave(amp, k, omega, t_anim)` | Superficie oceánica animada con perfil de Stokes |
| `ocean_thermocline(z_mixed, z_thick, T_surface, T_deep, T_iso)` | Capa de mezcla con termoclina |
| `cyclone_sdf(r_eye, r_outer, t_anim)` | Vórtice ciclónico axisimétrico animado |

## Notas
- La capa oceánica dentro de esta librería es básica; la librería `std_geo_ocean.hm` profundiza en dinámica oceánica y mareas.
- El modelo usa la atmósfera estándar ICAO solo hasta la tropopausa (~11 km).
- Las SDFs de nubes y ciclones están diseñadas para su uso directo en renderizado basado en distancia (ray marching).
