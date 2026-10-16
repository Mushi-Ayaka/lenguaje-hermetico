# std_astro_cosmology.hm

**Capa:** 8.3 — Cosmología Física
**Versión:** 0.2
**Fuente:** Métrica FLRW, Ecuaciones de Friedmann, Ley de Hubble
**Dependencias:** std_math_core.hm, std_phys_relativity.hm
**Fecha:** 6 de junio de 2026

## Cómo
Implementa la ley de Hubble-Lemaître, el desplazamiento al rojo cosmológico, la primera ecuación de Friedmann (universo plano k=0 con constante cosmológica) y la temperatura del CMB en función del factor de escala. Incluye SDFs para la red cósmica de filamentos (cosmic web) y burbujas de expansión inflacionarias.

## Por qué
Proporciona las ecuaciones fundamentales de la cosmología moderna para simular la expansión del universo, la evolución del CMB y las estructuras a gran escala.

## Para qué
Calcular velocidades de recesión, desplazamientos al rojo, densidad crítica y temperatura del CMB en cualquier época cosmológica; generar visualizaciones de la red cósmica y burbujas de inflación.

## Funciones principales

| Función | Descripción |
|---------|-------------|
| `hubble_velocity(H0, distance)` | Velocidad de recesión por la ley de Hubble-Lemaître |
| `cosmological_redshift(lambda_obs, lambda_em)` | Desplazamiento al rojo cosmológico `z` |
| `friedmann_H_squared(G_const, density, Lambda_cosmo, c_light)` | Primera ecuación de Friedmann (universo plano, k=0) |
| `cmb_temperature(T0_current, scale_factor_a)` | Temperatura del CMB en función del factor de escala |

## SDFs definidas

| SDF | Descripción |
|-----|-------------|
| `cosmic_web_filaments(sc_val, thickness)` | Filamentos de la red cósmica basados en Voronoi 3D |
| `inflation_bubble(r_initial, expansion_rate, t_time)` | Burbuja de expansión inflacionaria (radio creciente exponencial) |

## Notas
- La ecuación de Friedmann implementada asume curvatura espacial cero (k=0) y no separa densidad de materia, radiación y energía oscura.
- La red cósmica se modela con Voronoi 3D, una aproximación geométrica a la distribución de materia a gran escala.
- Depende de `std_phys_relativity.hm` para constantes relativistas.
- Es la librería de más alto nivel en la jerarquía de capas de Astrofísica.
