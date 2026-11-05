# std_chem_optics.hm

**Capa:** Química (Capa 4 — Óptica Química: Fresnel, BRDF, refracción)
**Versión:** 1.2
**Fuente:** Schlick "Ideal Reflectance", Cook-Torrance "Microfacet Theory"
**Dependencias:** std_math_core.hm
**Fecha:** 6 de junio de 2026 (reescrita 2026-10-23, optimizada 2026-11-04)

## Cómo
Implementa un pipeline de renderizado físico-químico: reflectancia de Fresnel (exacta S/P, no polarizada, Schlick, compleja para metales), modelos de BRDF (Lambert, Phong, Blinn-Phong, Cook-Torrance), distribuciones de microfacetas (GGX, Beckmann), funciones de geometría (Smith, Schlick-GGX), absorción y dispersión (Beer-Lambert, Rayleigh, Mie, Cauchy, Sellmeier), reflectancia de materiales (metal, dieléctrico, plástico, vidrio, agua, piel), efectos especiales (película delgada, anisotropía, clear coat, sheen, translucencia, Fabry-Pérot, difracción, cristal fotónico) y SDFs de relieve para visualizar esas propiedades.

El lenguaje no tiene strings, arrays, `for` ni `if`: los materiales se eligen con **códigos numéricos** y las compuertas se implementan con `step`. Los BRDF son escalares: el llamador calcula los cosenos directores.

## Por qué
Conecta la composición química de un material con su apariencia visual. Es la base para renderizado fotorrealista y simulación óptica de materiales.

## Para qué
- Calcular reflectancia de Fresnel para cualquier interfaz (dieléctrica o metálica)
- Modelar BRDFs realistas (Cook-Torrance con GGX)
- Simular dispersión cromática (Cauchy, Sellmeier) y scattering atmosférico
- Renderizar materiales específicos (metales, dieléctricos, plásticos, vidrio, agua, piel)
- Generar SDFs de efectos ópticos (película delgada, difracción, cristales fotónicos)

## Convenciones numéricas
| Parámetro | Valores |
|---|---|
| `metal_index` | 0=Ag 1=Au 2=Cu 3=Al |
| `cos_theta` / `N_dot_X` | coseno director en [0,1] (lo calcula el llamador) |
| `rough` / `a_rough` | rugosidad en [0,1]; `a_rough` es α de GGX (se eleva al cuadrado dentro) |
| `wavelength` | nanómetros; las constantes de metal están referidas a 600 nm |
| `theta1`, `phase` | radianes |
| `sss`, `metal_amount`, `specular_strength` | fracción en [0,1] |
| `band_width` | semiancho 1/e de la banda gaussiana (no σ) |
| `F0` | reflectancia normal de la interfaz; para dieléctricos usar `dielectric_f0(n)` |

## Funciones principales
| Función | Devuelve | Descripción |
|---|---|---|
| `ior_metal_index(metal_index)` | n | 0.05 (Ag), 0.20 (Au), 0.62 (Cu), 1.44 (Al) @ 600 nm |
| `ior_metal_extinction(metal_index)` | k | 3.00 (Ag), 2.62 (Au), 3.42 (Cu), 5.23 (Al) @ 600 nm |
| `fresnel_exact_s(n1, n2, theta1)` | 0–1 | Fresnel exacto S-polarizado |
| `fresnel_exact_p(n1, n2, theta1)` | 0–1 | Fresnel exacto P-polarizado |
| `fresnel_unpolarized(n1, n2, theta1)` | 0–1 | Promedio S/P con una sola refracción |
| `schlick_fresnel(F0, cos_theta)` | 0–1 | F0 + (1−F0)(1−cosθ)⁵ |
| `fresnel_complex(n, k, cos_theta)` | 0–1 | Fresnel para metales con coeficiente k |
| `conductor_fresnel(F0, cos_theta, rough)` | 0–1 | Schlick modulado por rugosidad |
| `dielectric_f0(ior_value)` | 0–1 | ((n−1)/(n+1))² |
| `lambert_brdf(rho_diffuse)` | 1/sr | ρ/π |
| `phong_brdf(ks, shininess, R_dot_V)` | 1/sr | (n+2)/(2π)·ks·(R·V)ⁿ |
| `blinn_phong_brdf(ks, shininess, N_dot_H)` | 1/sr | (n+8)/(8π)·ks·(N·H)ⁿ |
| `cook_torrance_brdf(D, G, F, N_dot_L, N_dot_V)` | 1/sr | D·G·F / (4(N·L)(N·V)) |
| `ggx_distribution(a_rough, N_dot_H)` | 1/sr | α²/(π(h²(α²−1)+1)²) |
| `beckmann_distribution(a_rough, N_dot_H)` | 1/sr | exp(−tan²θ/α²)/(π α² cos⁴θ) |
| `smith_ggx_geometry(a_rough, N_dot_X)` | 0–1 | G1 de Smith con GGX |
| `schlick_ggx_geometry(a_rough, N_dot_X)` | 0–1 | Aproximación de Schlick-GGX (k=α/2) |
| `beer_lambert_absorption(I0, mu, dist)` | intensidad | I0·e^(−μd) |
| `spectral_absorption_coefficient(λ, centro, ancho, fuerza)` | 1/longitud | Gaussiana de pico `fuerza` |
| `rayleigh_scattering_coefficient(λ, base)` | 1/longitud | base·(550/λ)⁴ |
| `mie_scattering_coefficient(λ, base, size_parameter)` | 1/longitud | base·(550/λ)^p |
| `atmospheric_scattering(λ, rayleigh, mie)` | 1/longitud | Rayleigh + Mie (p=0.84) |
| `cauchy_dispersion_law(λ_nm, A, B, C)` | índice | A + B/λ² + C/λ⁴ (λ en µm) |
| `cauchy_dispersion_default(λ_nm)` | índice | Vidrio común (A=1.5, B=0.004, C=0.0001) |
| `sellmeier_dispersion_law(λ_nm, B1,B2,B3, C1,C2,C3)` | índice | sqrt(1+Σ Bᵢλ²/(λ²−Cᵢ)) |
| `metal_reflectance(F0, cosθ, rough, λ)` | 0–1 | Conductor + modulación espectral (1 en 600 nm) |
| `dielectric_reflectance(ior, cosθ, sss)` | 0–1 | Fresnel desde el IOR + término SSS |
| `plastic_reflectance(color, ks, rough, cosθ)` | 0–1 | Difuso (1−ks) + especular Schlick |
| `glass_reflectance(ior, espesor, μ, cosθ)` | 0–1 | Reflexión + transmisión absorbida |
| `water_reflectance(profundidad, turbidez, cosθ, λ)` | 0–1 | F0=0.02 + atenuación de dos bandas |
| `skin_reflectance(difuso, sss_color, espesor, cosθ)` | 0–1 | Piel con SSS (pesos 0.6/0.3/0.03) |
| `thin_film_interference(espesor, ior, cosθ, λ)` | 0–1 | 0.5 + 0.5·cos(2π(2nd·cosθ/λ + 0.5)) |
| `anisotropic_reflectance(rough, aniso, T·H, B·H)` | 1/sr | GGX(αx)·GGX(αy)·0.5 |
| `clear_coat_reflectance(base, ior_barniz, cosθ)` | 0–1 | F_coat + (1−F_coat)·base |
| `sheen_reflectance(color, rough, cosθ)` | 0–1 | Lóbulo rasante para tejidos |
| `translucency_transmission(espesor, μs, μa)` | 0–1 | e^(−d(μs+μa)) |
| `fabry_perot_transmission(L, R, λ, cosθ)` | 0–1 | Airy: T_max/(1+F·sin²(δ/2)) |
| `surface_sagitta(R, r)` | longitud | R − sqrt(R²−r²) |
| `smooth_gate(edge0, edge1, v)` | 0–1 | smoothstep manual con `sat` |

### Helpers internos (`*_field`)
Precalculan con `let` la raíz radial, la longitud de onda y los términos de Fresnel una sola vez por punto, para que las SDF no repitan subexpresiones. No están pensados como API de usuario, pero son invocables: `cook_torrance_field`, `cauchy_dispersion_field`, `thin_film_field`, `spectral_absorption_field`, `chromatic_lens_field`, `fabry_perot_field`, `grating_field`, `photonic_crystal_field`.

## SDFs definidas (relieves; `sdf = y − altura − umbral`)
| SDF | Descripción |
|---|---|
| `cook_torrance_surface(rough, metal, tint, N·V, N·L, N·H, umbral)` | Altura = BRDF Cook-Torrance con GGX + Smith |
| `cauchy_dispersion_material(A, B, C, espesor, rango_λ, fuerza)` | Película delgada cuyo IOR y λ varían con el radio |
| `thin_film_interference_surface(espesor, ior, λ0, umbral)` | Interferencia con cosθ = 1/sqrt(1+r²) |
| `spectral_absorption_material(c1, w1, s1, c2, w2, s2, profundidad, umbral)` | Absortancia de dos bandas gaussianas |
| `anisotropic_material_surface(rough, aniso, T·H, B·H, umbral)` | Metal cepillado (GGX anisotrópico) |
| `subsurface_scattering_material(μs, μa, espesor, umbral)` | Translucencia con caída radial gaussiana |
| `chromatic_lens_dispersion(R, espesor, A, B, C)` | Lente con 3 sagittas (700/550/450 nm) |
| `fabry_perot_cavity(L, R_espejo, λ, umbral)` | Transmisión Airy con cosθ = L/sqrt(L²+r²) |
| `diffraction_grating(periodo, profundidad, λ, orden, umbral)` | Ranuras suaves + franjas cosenoidales |
| `photonic_crystal(a, r_agujero, contraste, umbral)` | Un agujero centrado por celda (periodicidad vía `mod_unit`) |

## Corregido 2026-11-04
- **Pase de optimización (v1.2)** sobre la reescritura v1.1:
  - Exponentes fijos por multiplicaciones encadenadas: Schlick⁵ (`c2*c2*c`), Rayleigh⁴ (`r2*r2`), sin² de Fabry-Pérot, `t²` de GGX y Beckmann. `pow` queda solo donde el exponente es variable (Phong, Blinn-Phong, Mie).
  - Helpers `*_field` con `let`: la raíz radial, λ, los IOR de Cauchy y los términos de Fresnel se calculan una vez por punto en lugar de repetirse en cada SDF.
  - `fresnel_unpolarized` calcula una sola refracción (antes llamaba a S y P, repitiendo `asin/sin/cos`).
  - `photonic_crystal`: de 12 `mod_unit` repetidos y 3 agujeros solapados a **un agujero centrado por celda**; la periodicidad la da `mod_unit` y el relieve baja dentro del agujero.
  - Eliminados el parámetro muerto `clear_coat_thickness` y la constante `F0_METAL_BASE`.
- **Correcciones funcionales:**
  - `dielectric_reflectance`, `glass_reflectance` y `clear_coat_reflectance` derivan F0 del IOR con `dielectric_f0` (antes ignoraban `ior_value` y usaban 0.04 fijo).
  - `cook_torrance_surface` pasaba `rough²` a GGX/Smith, que ya elevan al cuadrado (daba α⁴); ahora pasa `rough`.
  - `thin_film_interference_surface` y `fabry_perot_cavity` usaban el azimut `atan(z,x)`; ahora usan cosenos de visión físicos (`1/sqrt(1+r²)` y `L/sqrt(L²+r²)`).
  - `ior_metal_extinction`: Cu corregido de 2.62 a 3.42 (compuerta extra); Al queda en 5.23.
  - `metal_reflectance`: modulación espectral centrada en 600 nm (antes `sin(λ·0.01)` desviaba la referencia).
  - `skin_reflectance`: pesos 0.6/0.3/0.03; antes la suma podía superar 1.
  - `translucency_transmission`: exponenciales combinadas, sin división intermedia.
- Dependencias reducidas a `std_math_core.hm` (la v1.0 listaba módulos inexistentes o no usados).

## Verificado 2026-11-04
- Compila: exit 0 (`libreria/std_chem_optics.hm`). Probe de 10 defs: exit 0, **10 customs** exactos, `.rih` sin `for`/`if`/`let`/`atan2`/`union_array`/`append`/`?` residuales.
- **49/49 valores de funciones OK** (`optics_checks.ps1`, evaluando la expresión emitida en `.rih` contra una segunda implementación en PowerShell): `ior_index(0)=0.05`, `ior_index(3)=1.44`, `ior_k(2)=3.42`, `fresnel_s(1,1.5,0.5)=0.0559419`, `fresnel_p=0.0265413`, `unpolarized=0.0412416`, `schlick(0.04,0.5)=0.07`, `fresnel_complex(1.5,3,0.8)=0.605635`, `conductor(0.9,0.5,0.3)=0.902656`, `dielectric_f0(1.5)=0.04`, `lambert(0.8)=0.254648`, `phong(0.5,20,0.8)=0.0201842`, `blinn(0.5,20,0.9)=0.0677233`, `cook(0.3,0.5,0.04,0.8,0.7)=0.00267857`, `ggx(0.3,0.8)=0.164275`, `beckmann=0.0166691`, `smith=0.987654`, `schlick_ggx=0.963854`, `beer(1,0.5,2)=0.367879`, `spectral_abs(550,500,50,0.8)=0.294304`, `rayleigh(450,1)=2.23152`, `mie(550,1,1.5)=1`, `atmospheric=1.5`, `cauchy(550)=1.514316`, `sellmeier(550)=1.53602`, `metal(0.9,0.5,0.3,600)=0.902656`, `dielectric(1.5,0.5,0.2)=0.163`, `plastic(0.8,0.3,0.5,0.6)=0.574949`, `glass(1.5,0.01,0.5,0.6)=0.522546`, `water(1,0.2,0.6,450)=0.28812`, `skin(0.8,0.6,0.1,0.5)=0.649124`, `thin_film(0.3,1.5,0.6,550)=9.5139e−6`, `anisotropic(0.3,0.7,0.8,0.6)=0.000931644`, `clear_coat(0.5,1.5,0.6)=0.524915`, `sheen(0.7,0.4,0.6)=0.0609288`, `translucency(0.1,0.5,0.2)=0.932394`, `fabry(1,0.9,0.55,1)=9.26837e−6`, `sagitta(2,1)=0.267949`, `smooth_gate(0,1,0.5)=0.5`, más los 8 helpers `*_field` en (x,z)=(0,0) o muestras radiales.
- **10/10 puntos SDF OK** (evaluados con `y=0`; el valor es `−altura−umbral`): Cook-Torrance +0.298658 (altura negativa), Cauchy −4.9626e−6, película delgada −0.300002, absorción espectral −0.224207, anisotrópico −0.400932, SSS −0.765525, lente cromática +0.0474235, Fabry-Pérot −0.300008, red de difracción −0.7, cristal fotónico −0.274115.
- Sin artefactos residuales (probes y `.rih` eliminados).

## Notas
V1.2. Las constantes ópticas son valores típicos a 600 nm y los modelos de materiales son aproximaciones visuales, no medidas espectroscópicas. El resolver sustituye parámetros textualmente: pasar expresiones complejas (no valores ya calculados) a una SDF que repite el parámetro multiplica el tamaño del `.rih`; los helpers `*_field` reducen ese coste dentro de la librería, pero el llamador debería precalcular valores para las SDF.
