# std_astro_stellar.hm

**Capa:** 8.2 — Astrofísica Estelar
**Versión:** 0.2
**Fuente:** Equilibrio hidrostático, Fusión p-p, Límite de Eddington
**Dependencias:** std_math_core.hm, std_phys_thermo.hm
**Fecha:** 6 de junio de 2026

## Cómo
Modela la estructura estelar interna mediante equilibrio hidrostático (gradiente de presión radial), el límite de luminosidad de Eddington y las tasas de generación de energía por fusión (cadena protón-protón y ciclo CNO). Incluye SDFs para representar estrellas con corona y púlsares con chorros relativistas.

## Por qué
Permite simular la física fundamental del interior estelar para generación procedural de estrellas, evolución estelar y renderizado astronómico.

## Para qué
Calcular perfiles de presión en estrellas, luminosidad máxima antes de que la radiación desestabilice la estrella, tasas de fusión nuclear en función de temperatura y densidad, y representar visualmente estrellas, coronas y púlsares.

## Funciones principales

| Función | Descripción |
|---------|-------------|
| `hydrostatic_dp_dr(G_const, M_enclosed, density, r_distance)` | Gradiente de presión radial por equilibrio hidrostático |
| `eddington_luminosity(G_const, M_mass, m_proton, c_light, sigma_thomson)` | Límite de luminosidad de Eddington |
| `pp_chain_energy_rate(eps_0, density, X_hydrogen_frac, T_temp)` | Tasa de energía de cadena p-p (~T⁴) |
| `cno_cycle_energy_rate(eps_0_cno, density, X_hydrogen, X_cno, T_temp)` | Tasa de energía del ciclo CNO (~T¹⁹·⁹) |

## SDFs definidas

| SDF | Descripción |
|-----|-------------|
| `star_corona(r_star, corona_scale)` | Estrella esférica con corona de densidad exponencial |
| `pulsar_jets(r_star, jet_angle, jet_length)` | Púlsar con chorros cónicos bipolares |

## Notas
- Las tasas de fusión son aproximaciones de ley de potencias; la cadena p-p domina en estrellas como el Sol, el ciclo CNO domina en estrellas masivas.
- La SDF `star_corona` modela la corona como un halo de densidad decreciente, no como plasma físico.
- Depende de `std_phys_thermo.hm` para constantes termodinámicas.
