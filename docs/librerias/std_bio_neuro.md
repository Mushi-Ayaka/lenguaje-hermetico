# std_bio_neuro.hm

**Capa:** 6.5 — Neurobiología y Neurociencia Computacional
**Versión:** 0.2
**Fuente:** Integrate-and-Fire, Hodgkin-Huxley Simplificado, Regla de Hebb
**Dependencias:** std_math_core.hm, std_math_dynamics.hm
**Fecha:** 6 de junio de 2026

## Cómo
Implementa modelos neuronales de dinámica de membrana (Integrate-and-Fire, Izhikevich), reglas de plasticidad sináptica (Hebb, Oja), activación tipo perceptrón (sigmoide) y SDFs de estructuras neuronales (soma, axón mielinizado). Los modelos Izhikevich capturan dinámicas de disparo neuronal con bajo costo computacional.

## Por qué
La neurobiología explica cómo el sistema nervioso procesa información. Estos modelos son la base de la neurociencia computacional y permiten desde simulaciones de una sola neurona hasta redes neuronales artificiales.

## Para qué
Simular potenciales de acción y disparo neuronal, modelar plasticidad sináptica dependiente de actividad (aprendizaje hebbiano), y visualizar estructuras neuronales en 3D (soma, dendritas, axón con mielina).

## Funciones principales
- `integrate_and_fire_dv(V, I, Rm, Cm)` — Derivada del voltaje Integrate-and-Fire
- `spike_reset(V, V_thresh, V_rest)` — Reseteo condicional de voltaje
- `izhikevich_dv(v, u, I)` — Derivada del voltaje Izhikevich
- `izhikevich_du(v, u, a, b)` — Derivada de la variable de recuperación Izhikevich
- `hebbian_plasticity(η, x, y)` — Regla de Hebb (Δw)
- `oja_rule(η, x, y, w)` — Regla de Oja con normalización
- `sigmoid_activation(x)` — Función de activación sigmoide

## SDFs definidas
- `neuron_soma(r_soma)` — Cuerpo neuronal con rugosidad superficial sinusoidal
- `myelinated_axon(r_axon, myelin_thick, segment_len, gap_len)` — Axón con vaina de mielina segmentada

## Notas
- El modelo Integrate-and-Fire no incluye periodo refractario; el reseteo se maneja con `spike_reset`.
- Izhikevich requiere integración numérica externa (Runge-Kutta) para simulación temporal completa.
- La SDF del soma modula el radio con una perturbación sinusoidal 3D para dar textura.
- La mielina se representa como segmentos alternados a lo largo del axón usando `step` y `frac`.
