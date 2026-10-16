# std_phys_kinematics.hm

**Capa:** 2.1 — Mecánica Clásica (Cinemática y Dinámica)
**Versión:** 0.2
**Fuente:** Newton, Lagrange, Hamilton — mecánica analítica
**Dependencias:** std_math_core.hm, std_math_algebra.hm, std_math_calculus.hm
**Fecha:** 6 de junio de 2026

## Cómo
Proporciona funciones cerradas para movimiento rectilíneo uniformemente acelerado, caída libre, frenado, movimiento circular, péndulos, colisiones elásticas, trayectorias balísticas y órbitas elípticas.

## Por qué
La cinemática y dinámica clásicas son la base de cualquier simulación física. Esta librería encapsula las ecuaciones analíticas fundamentales para que puedan ser usadas directamente en SDFs y escenas sin resolver EDOs numéricamente.

## Para qué
- Simular proyectiles, péndulos, órbitas y colisiones en tiempo real
- Generar trayectorias y campos de movimiento para visualización artística
- Modelar sistemas mecánicos dentro del Lenguaje Hermético

## Funciones principales

| Función | Descripción |
|---|---|
| `pos_accel(x0, v0, a, t)` | Posición bajo aceleración constante: x₀ + v₀t + ½at² |
| `vel_accel(v0, a, t)` | Velocidad bajo aceleración constante |
| `freefall_y(y0, v0y, g, t)` | Caída libre: y₀ + v₀yt − ½gt² |
| `braking_distance(v0, a)` | Distancia de frenado: v₀²/(2\|a\|) |
| `circular_x(r, ω, t, φ₀)` / `circular_y(r, ω, t, φ₀)` | Posición en MCU |
| `angular_vel(freq)` | Velocidad angular desde frecuencia: 2πf |
| `centripetal_accel(ω, r)` | Aceleración centrípeta: ω²r |
| `centrifugal_force(m, ω, r)` | Fuerza centrífuga: mω²r |
| `rotational_energy(I, ω)` | Energía cinética rotacional: ½Iω² |
| `angular_momentum(I, ω)` | Momento angular: Iω |
| `pendulum_angle(A, ω, t, φ₀)` | Ángulo de péndulo simple |
| `pendulum_ω(g, L)` | Frecuencia angular de péndulo |
| `pendulum_period(g, L)` | Período de péndulo |
| `elastic_v1/v2(m₁, m₂, v₁, v₂)` | Velocidades post-colisión elástica 1D |
| `restitution(v₁', v₂', v₁, v₂)` | Coeficiente de restitución |
| `projectile_range(v₀, θ, g)` | Alcance máximo de proyectil |
| `projectile_max_height(v₀, θ, g)` | Altura máxima de proyectil |
| `orbit_r(p, e, θ)` | Radio orbital en cónica: p/(1+e·cosθ) |

## SDFs definidas

| SDF | Descripción |
|---|---|
| `projectile_trajectory(v0, θ, g, thick)` | Tubo delgado alrededor de la trayectoria parabólica |
| `pendulum_sdf(L, A, freq)` | Posición del péndulo en tiempo animado `w` |
| `circular_orbit_trail(r, ω, thick)` | Anillo orbital (diferencia de círculos) |

## Notas
- Usa `EPSILON` para evitar divisiones por cero en todas las funciones con denominadores.
- Las SDFs usan `custom()` para definir formas implícitas a partir de las ecuaciones cinemáticas.
- `DEG2RAD` se usa en `projectile_trajectory` para convertir ángulos de entrada.
