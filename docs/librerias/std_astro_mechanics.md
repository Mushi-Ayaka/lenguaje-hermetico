# std_astro_mechanics.hm

**Capa:** 8.1 — Mecánica Celeste y Órbitas
**Versión:** 0.2
**Fuente:** Leyes de Kepler, Límite de Roche, Problema de los 2 Cuerpos
**Dependencias:** std_math_core.hm, std_phys_kinematics.hm
**Fecha:** 6 de junio de 2026

## Cómo
Implementa la dinámica orbital avanzada: tercera ley de Kepler generalizada, velocidad de escape, límite de Roche para cuerpos rígidos y la ecuación vis-viva para velocidad orbital en cualquier punto de la órbita. Incluye SDFs para anillos planetarios y asteroides irregulares.

## Por qué
Proporciona las herramientas fundamentales de mecánica celeste para simular sistemas planetarios, órbitas de satélites y estabilidad de cuerpos en el espacio.

## Para qué
Calcular periodos orbitales, velocidades de escape, límites de ruptura por marea, generar anillos planetarios y asteroides procedurales para visualización astronómica.

## Funciones principales

| Función | Descripción |
|---------|-------------|
| `kepler_period(a_semi_major, M1, M2, G_const)` | Periodo orbital por la 3ª ley de Kepler generalizada |
| `escape_velocity(M_mass, r_distance, G_const)` | Velocidad de escape desde una distancia dada |
| `roche_limit_rigid(R_primary, rho_primary, rho_satellite)` | Límite de Roche para satélite rígido |
| `vis_viva_velocity(G_const, M_mass, r_distance, a_semi_major)` | Velocidad orbital en cualquier punto (ecuación vis-viva) |

## SDFs definidas

| SDF | Descripción |
|-----|-------------|
| `planetary_rings(r_inner, r_outer, thickness)` | Anillos planetarios (disco plano con agujero interior) |
| `asteroid_shape(rx, ry, rz, noise_amp, sc_val)` | Asteroide irregular elipsoidal con ruido FBM 3D |

## Notas
- La velocidad de escape no incluye correcciones relativistas; es la fórmula newtoniana clásica.
- El límite de Roche asume cuerpo rígido; para cuerpo fluido el factor cambia (~2.44 vs ~1.26).
- Los anillos planetarios son geométricamente planos; no incluyen inclinación ni excentricidad.
