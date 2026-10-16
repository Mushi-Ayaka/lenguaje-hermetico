# std_phys_relativity.hm

**Capa:** 2.8 — Relatividad General y Especial
**Versión:** 0.2
**Fuente:** Einstein (1905, 1916), Schwarzschild (1916)
**Dependencias:** std_math_core.hm, std_phys_sdf.hm
**Fecha:** 6 de junio de 2026

## Cómo
Implementa la relatividad especial (factor γ, contracción de Lorentz, dilatación temporal, adición de velocidades, efecto Doppler, intervalo de Minkowski) y la relatividad general (radio de Schwarzschild, factor de curvatura temporal, redshift gravitacional, lente gravitacional).

## Por qué
La relatividad describe el espacio-tiempo curvo y los efectos relativistas a altas velocidades. En el Lenguaje Hermético permite generar geometrías distorsionadas por la gravedad y objetos relativistas.

## Para qué
- Visualizar horizontes de eventos y embudos gravitacionales
- Modelar contracción de Lorentz en objetos en movimiento
- Simular lentes gravitacionales y redshift

## Funciones principales

| Función | Descripción |
|---|---|
| `β(v, c)` | Beta relativista: v/c |
| `γ_factor(β)` | Factor de Lorentz: 1/√(1−β²) |
| `rel_length(L₀, β)` | Contracción de Lorentz: L₀·√(1−β²) |
| `rel_time(t₀, β)` | Dilatación temporal: t₀/√(1−β²) |
| `rel_energy(m, β, c)` | Energía relativista: γmc² |
| `vel_addition(v, u, c)` | Adición relativista de velocidades |
| `doppler_rel(f₀, β)` | Efecto Doppler relativista |
| `minkowski_ds²(dt, dx, dy, dz, c)` | Intervalo espacio-temporal de Minkowski |
| `r_schwarzschild(M, G, c)` | Radio de Schwarzschild: 2GM/c² |
| `schwarzschild_time_factor(r, r_s)` | Factor de curvatura temporal: √(1 − r_s/r) |
| `gravitational_redshift(f₀, r, r_s)` | Redshift gravitacional |
| `gravitational_lensing_angle(G, M, b, c)` | Ángulo de desviación de luz: 4GM/(c²b) |
| `curved_potential(r, r_s)` | Potencial newtoniano generalizado |

## SDFs definidas

| SDF | Descripción |
|---|---|
| `black_hole_event_horizon(r_s)` | Horizonte de eventos de Schwarzschild (esfera) |
| `lorentz_contracted_box(w, h, d, β)` | Caja con contracción relativista en eje X |
| `gravity_funnel(M, sc_val)` | Embudo gravitacional 2D (curvatura espacial) |

## Notas
- Usa `SCHWARZSCHILD_SCALE = 2.0` como constante interna (r_s = 2GM/c² con M=G=c=1).
- `SPEED_OF_LIGHT` está normalizado (=1) desde `std_phys_sdf.hm`.
- Las funciones usan `max(EPSILON, ...)` para evitar raíces de números negativos.
