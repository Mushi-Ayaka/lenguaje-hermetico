# std_math_dynamics.hm

**Capa:** 1.5 — Teoría de la Información y Sistemas Dinámicos
**Versión:** 0.2
**Fuente:** Lorenz, Poincaré, Lyapunov, Shannon
**Dependencias:** std_math_core.hm, std_math_calculus.hm
**Fecha:** 6 de junio de 2026

## Cómo

`std_math_dynamics.hm` implementa sistemas dinámicos caóticos, mapas iterados y teoría de la información como funciones algebraicas cerradas. Incluye el atractor de Lorenz (derivadas dx, dy, dz), el atractor de Rössler, el exponente de Lyapunov local, mapas iterados (logístico, Hénon, Ikeda), sistemas de reacción-difusión (Gray-Scott, Fitzhugh-Nagumo), teoría de la información (entropía binaria de Shannon, capacidad de canal, complejidad de Kolmogorov aproximada) y SDFs de sistemas dinámicos (atractor de Lorenz como trayectoria en el tiempo, toro caótico).

## Por qué

Conecta las matemáticas puras (capas 1.1–1.3) con sistemas complejos: caos determinista, autoorganización, patrones emergentes y medición de información. Es el fundamento teórico para generar comportamientos orgánicos, impredecibles pero deterministas, esenciales para arte generativo y simulación de fenómenos naturales.

## Para qué

Se usa para generar animaciones con comportamiento caótico (mariposas de Lorenz, espirales de Rössler), crear texturas procedurales basadas en mapas iterados (Hénon, Ikeda), simular patrones de reacción-difusión tipo Gray-Scott (manchas, ondas químicas), modelar neuronas simplificadas (Fitzhugh-Nagumo), medir entropía y capacidad de canal en señales generadas, y construir SDFs animadas que siguen trayectorias caóticas o se deforman con dinámica no-lineal.

## Funciones principales

| Nombre | Descripción breve |
|---|---|
| `lorenz_dx(lx, ly, σ)` | dx/dt = σ(y − x) |
| `lorenz_dy(lx, ly, lz, ρ)` | dy/dt = x(ρ − z) − y |
| `lorenz_dz(lx, ly, lz, β)` | dz/dt = xy − βz |
| `rossler_dx(ry, rz)` | dx/dt = −y − z |
| `rossler_dy(rx, ry, a)` | dy/dt = x + ay |
| `rossler_dz(rx, rz, b, c)` | dz/dt = b + z(x − c) |
| `local_lyapunov(df_dx, df_dy)` | Exponente de Lyapunov local: ½ln(dfx² + dfy²) |
| `logistic_map(x, r)` | Mapa logístico: rx(1−x) |
| `henon_x(hx, hy, a)` | Mapa de Hénon: 1 − ax² + y |
| `henon_y(hx, b)` | Mapa de Hénon: bx |
| `ikeda_t(ix, iy)` | Ángulo t del mapa de Ikeda |
| `ikeda_x(ix, iy, μ)` | x' del mapa de Ikeda |
| `ikeda_y(ix, iy, μ)` | y' del mapa de Ikeda |
| `gray_scott_du(u, v, ∇²u, Du, F)` | ∂u/∂t Gray-Scott: Du∇²u − uv² + F(1−u) |
| `gray_scott_dv(u, v, ∇²v, Dv, F, k)` | ∂v/∂t Gray-Scott: Dv∇²v + uv² − (F+k)v |
| `fhn_dv(V, W, I_ext)` | ∂V/∂t Fitzhugh-Nagumo |
| `fhn_dw(V, W, a, b, τ)` | ∂W/∂t Fitzhugh-Nagumo |
| `binary_entropy(p)` | Entropía de Shannon binaria: H = −p·log₂p − (1−p)·log₂(1−p) |
| `channel_capacity(W, SNR)` | Capacidad de canal: C = W·log₂(1 + SNR) |
| `kolmogorov_complexity_approx(H, L)` | Complejidad de Kolmogorov estimada: H · L |

## SDFs definidas

| Nombre | Descripción |
|---|---|
| `lorenz_attractor_point(σ, ρ, β, sc)` | Punto animado sobre la trayectoria de Lorenz en tiempo w |
| `chaotic_torus(R, r, chaos_amp, chaos_freq)` | Toro con deformación caótica (espacio de fases) |

## Notas

- Los sistemas dinámicos se expresan como derivadas: la integración temporal debe realizarse en la escena `.herm` (no hay integrador numérico en la librería).
- Los mapas iterados (logístico, Hénon, Ikeda) son funciones algebraicas de un paso; la iteración debe manejarse externamente.
- `binary_entropy` usa LN2 para cambio de base a log₂ mediante log() nativo.
- `local_lyapunov` es una estimación puntual; el exponente global requiere promediar sobre toda la trayectoria.
- Las SDFs usan la variable w para representar el tiempo, permitiendo animación.
