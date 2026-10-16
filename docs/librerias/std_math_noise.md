# std_math_noise.hm

**Capa:** 1.4 — Teoría de la Probabilidad y Ruido Procedural
**Versión:** 0.2
**Fuente:** Perlin (1985), Worley (1996), FBM procedural noise
**Dependencias:** std_math_core.hm
**Fecha:** 5 de junio de 2026

## Cómo

`std_math_noise.hm` implementa ruido procedural completamente determinista sobre el hash de `std_math_core.hm`. Incluye value noise 1D/2D/3D (interpolación smootherstep de hashes), FBM (Fractional Brownian Motion) en 2D y 3D con 4 y 6 octavas, turbulencia FBM 3D, ruido celular Voronoi 2D (Worley), patrones geométricos procedurales (grid, dots, checker 2D/3D), domain warping 2D (Quilez) y SDFs con ruido procedural (esfera ruidosa, terreno FBM, nube volumétrica).

## Por qué

El ruido procedural es la técnica fundamental para generar texturas orgánicas, terrenos, nubes y superficies naturales sin usar texturas de imagen. Hermético, al no tener muestreo de texturas, necesita ruido algebraico puro. Esta librería transforma hashes deterministas en paisajes Continuos y suaves listos para esculpir SDFs.

## Para qué

Se usa para dar apariencia orgánica a formas geométricas (esferas rugosas, montañas, nubes), generar texturas procedurales (mármol, madera, fuego) mediante FBM y domain warping, crear patrones repetitivos (grid, damier, puntos) para escenas abstractas, construir terrenos infinitos con `terrain_fbm`, modelar volúmenes de nubes con `cloud_volume`, y distorsionar el espacio de coordenadas para efectos de deformación artística.

## Funciones principales

| Nombre | Descripción breve |
|---|---|
| `vnoise1d(x)` | Value noise 1D suave |
| `vnoise2d(x, y)` | Value noise 2D con interpolación bilineal |
| `vnoise3d(x, y, z)` | Value noise 3D con interpolación trilineal |
| `fbm2d_4(x, y)` | FBM 2D: 4 octavas, lacunarity 2, gain 0.5 |
| `fbm3d_4(x, y, z)` | FBM 3D: 4 octavas |
| `fbm3d_6(x, y, z)` | FBM 3D: 6 octavas (más detalle) |
| `turbulence3d_4(x, y, z)` | FBM turbulento 3D (abs para arrugas) |
| `voronoi2d(x, y)` | Ruido celular Voronoi 2D (distancia al vecino más cercano) |
| `grid_pattern(x, y, cell, width)` | Patrón de cuadrícula |
| `dot_pattern(x, y, cell, radius)` | Patrón de puntos |
| `checker(x, y, size)` | Tablero de ajedrez 2D |
| `checker3d(x, y, z, size)` | Tablero de ajedrez 3D |
| `domain_warp2d(x, y, strength)` | Domain warping 2D (Quilez) con FBM |

## SDFs definidas

| Nombre | Descripción |
|---|---|
| `noisy_sphere(r, noise_scale, noise_strength)` | Esfera con desplazamiento FBM superficial |
| `terrain_fbm(scale, height)` | Plano con desplazamiento vertical FBM (montañas) |
| `cloud_volume(scale, density_threshold)` | Volumen implícito de nube con FBM 6 octavas |

## Notas

- El ruido es 100% determinista: mismas coordenadas producen exactamente el mismo valor siempre.
- FBM usa 4 o 6 octavas hardcodeadas (sin loops en Hermético). La frecuencia se duplica (lacunarity=2) y la amplitud se reduce a la mitad (gain=0.5) en cada octava.
- `voronoi2d` evalúa solo 4 celdas vecinas (no la vecindad completa de 9), lo que puede producir artefactos menores en los bordes de celda.
- `domain_warp2d` usa FBM como perturbación de las coordenadas de entrada del value noise, creando texturas más orgánicas y complejas.
- Las SDFs (noisy_sphere, terrain_fbm, cloud_volume) combinan formas canónicas con ruido FBM para deformación superficial o volumétrica.
