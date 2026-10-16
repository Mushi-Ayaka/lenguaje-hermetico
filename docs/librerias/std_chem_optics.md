# std_chem_optics.hm

**Capa:** Química (Capa 4 — Óptica Química: Fresnel, BRDF, refracción)
**Versión:** 1.0
**Fuente:** Schlick "Ideal Reflectance", Cook-Torrance "Microfacet Theory"
**Dependencias:** std_math_core.hm, std_math_algebra.hm, std_math_calculus.hm, std_bridge_spectral.hm, std_bridge_atomic.hm
**Fecha:** 6 de junio de 2026

## Cómo
Implementa un pipeline completo de renderizado físico-químico: reflectancia de Fresnel (exacta S/P, no polarizada, Schlick, compleja para metales, conductores), modelos de BRDF (Lambert, Phong, Blinn-Phong, Cook-Torrance), distribuciones de microfacetas (GGX, Beckmann), funciones de geometría (Smith, Schlick), absorción y dispersión (Beer-Lambert, Rayleigh, Mie, Cauchy, Sellmeier), reflectancia de materiales específicos (metal, dieléctrico, plástico, vidrio, agua, piel), efectos especiales (iridiscencia, anisotropía, clear coat, sheen, translucencia), y SDFs ópticas para visualización.

## Por qué
Conecta la composición química de un material con su apariencia visual. Esencial para renderizado fotorrealista y simulación óptica de materiales.

## Para qué
- Calcular reflectancia de Fresnel para cualquier interfaz
- Modelar BRDFs realistas (Cook-Torrance GGX)
- Simular dispersión cromática (Cauchy, Sellmeier)
- Renderizar materiales específicos (metales, dieléctricos, plásticos, vidrio, agua, piel)
- Generar SDFs de efectos ópticos (película delgada, difracción, cristales fotónicos)

## Funciones principales
| Función | Descripción |
|---|---|
| `fresnel_exact_s(n₁, n₂, θ)` | Fresnel exacto S-polarizado |
| `fresnel_exact_p(n₁, n₂, θ)` | Fresnel exacto P-polarizado |
| `schlick_fresnel(F₀, cosθ)` | Aproximación de Schlick |
| `fresnel_complex(n, k, cosθ)` | Fresnel para metales (con k) |
| `lambert_brdf(ρ)` | BRDF de Lambert (difuso perfecto) |
| `cook_torrance_brdf(D, G, F, N·L, N·V)` | BRDF de Cook-Torrance |
| `ggx_distribution(α, N·H)` | Distribución GGX/Trowbridge-Reitz |
| `smith_ggx_geometry(α, N·X)` | Función de geometría Smith GGX |
| `beer_lambert_absorption(I₀, μ, d)` | Ley de Beer-Lambert |
| `rayleigh_scattering(λ, base)` | Dispersión de Rayleigh (λ⁻⁴) |
| `cauchy_dispersion_law(λ, A, B, C)` | Dispersión de Cauchy |
| `sellmeier_dispersion_law(λ, B, C)` | Dispersión de Sellmeier |
| `thin_film_interference(d, n, cosθ, λ)` | Interferencia de película delgada |

## SDFs definidas
| SDF | Descripción |
|---|---|
| `cook_torrance_surface(roughness, metallic, color, threshold)` | Superficie con BRDF Cook-Torrance |
| `cauchy_dispersion_material(A, B, C, d, λ_range, strength)` | Material con dispersión de Cauchy |
| `thin_film_interference_surface(d, n, λ₀, threshold)` | Película delgada interferométrica |
| `spectral_absorption_material(bands, widths, strengths, depth, threshold)` | Material con absorción espectral |
| `anisotropic_material_surface(roughness, anisotropy, dir, threshold)` | Material anisotrópico (metal cepillado) |
| `subsurface_scattering_material(scatter, absorption, thickness, threshold)` | Material con SSS |
| `chromatic_lens_dispersion(R, d, A, B, C, aperture)` | Lente con aberración cromática |
| `fabry_perot_cavity(L, R_mirror, λ, F, threshold)` | Interferómetro Fabry-Pérot |
| `diffraction_grating(period, depth, λ, order, threshold)` | Red de difracción |
| `photonic_crystal(a, r_hole, contrast, threshold)` | Cristal fotónico |

## Notas
V1.0 con madurez. Constantes IOR para Ag, Au, Cu, Al a 600nm. Los efectos de materiales son aproximaciones visuales escaladas. Las SDFs ópticas convierten propiedades físicas en geometría.
