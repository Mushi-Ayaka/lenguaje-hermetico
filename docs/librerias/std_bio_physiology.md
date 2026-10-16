# std_bio_physiology.hm

**Capa:** 6.4 — Fisiología (Sistémica y Celular)
**Versión:** 0.2
**Fuente:** Guyton & Hall, Ecuaciones de Flujo de Poiseuille, Alometría de Kleiber
**Dependencias:** std_math_core.hm, std_phys_kinematics.hm
**Fecha:** 6 de junio de 2026

## Cómo
Modela hemodinámica mediante la ley de Poiseuille (flujo laminar en vasos), resistencia vascular periférica, número de Reynolds (transición a turbulencia), filtración capilar (ecuación de Starling), y metabolismo alométrico (ley de Kleiber). Las SDFs representan vasos sanguíneos como tubos y alvéolos pulmonares como cáscaras esféricas.

## Por qué
La fisiología integra los mecanismos físicos y químicos que permiten el funcionamiento de órganos y sistemas. Es el puente entre la biología celular y la medicina sistémica.

## Para qué
Simular flujo sanguíneo en vasos, calcular resistencias vasculares y riesgo de turbulencia, modelar intercambio de fluidos en capilares, estimar tasa metabólica basal por masa corporal, y visualizar estructuras fisiológicas en 3D.

## Funciones principales
- `poiseuille_flow(ΔP, r, η, L)` — Flujo volumétrico por ley de Poiseuille
- `vascular_resistance(η, L, r)` — Resistencia vascular periférica
- `reynolds_number(ρ, v, D, η)` — Número de Reynolds
- `starling_filtration(Kf, Pc, Pi, σ, πc, πi)` — Filtración capilar de Starling
- `kleiber_metabolism(mass, scale)` — Tasa metabólica basal alométrica

## SDFs definidas
- `blood_vessel(radius, thickness)` — Sección de vaso sanguíneo (tubo)
- `alveolus(inner_radius, wall_thickness)` — Alvéolo pulmonar (cáscara esférica)

## Notas
- La ley de Poiseuille asume flujo laminar, newtoniano y estacionario.
- El número de Reynolds > 2000 indica turbulencia potencial en vasos grandes.
- Starling usa `sigma_ref` como coeficiente de reflexión de proteínas.
- Kleiber usa exponente 3/4 (ley empírica para mamíferos); ajustar `scale_factor` por taxón.
