# std_chem_supramolecular.hm

**Capa:** Química (4.4 — Química Supramolecular)
**Versión:** 1.1
**Fuente:** "Supramolecular Chemistry" by Steed & Atwood
**Dependencias:** std_math_core.hm
**Fecha:** 6 de junio de 2026 (reescrita 2026-11-04)

## Cómo
Implementa modelos de química supramolecular: reconocimiento molecular (energía de enlace huésped-hospedador, constante de asociación, selectividad, complementariedad geométrica, energía de preorganización), autoensamblaje (criterio espontáneo, tasa de ensamblaje, estabilidad de arquitectura, cooperatividad, defectos), máquinas moleculares (eficiencia de motor, velocidad de rotación, ciclo de trabajo, switches, actuadores), propiedades de agregados (número de agregación, CMC, fase de cristal líquido, radio de vesícula, estabilidad de cápsula) y SDFs de arquitecturas supramoleculares (micela, vesícula liposomal, catenano, rotaxano, cavitando, autoensamblaje hexagonal).

El lenguaje no tiene strings, arrays, `for` ni `if`: los estados se devuelven como **códigos numéricos** y las compuertas se implementan con `step`/`min`/`max`.

## Por qué
La química supramolecular va más allá de la molécula individual hacia sistemas complejos autoensamblados. Es la base de la nanotecnología molecular y los materiales inteligentes.

## Para qué
- Modelar energías de interacción huésped-hospedador y constantes de asociación
- Calcular selectividad, complementariedad y preorganización
- Simular criterios de autoensamblaje espontáneo y formación de defectos
- Modelar máquinas moleculares (motores, switches, actuadores)
- Visualizar arquitecturas: micelas, vesículas, catenanos, rotaxanos, cavitandos, autoensamblajes hexagonales

## Convenciones numéricas
| Parámetro | Valores |
|---|---|
| `binding_energy`, `entropy_cost`, `defect_energy` | kJ/mol |
| `temperature` | kelvin (R = 8.314462618 J/(mol·K)) |
| `phase_code` (`liquid_crystal_phase`) | 0=isotrópico, 1=nemático, 2=esméctico |
| `current_state` / retorno de `molecular_switch_state` | 0 o 1 |
| `rim_functionalities` | número entero 0..8 (grupos de borde visibles) |
| `aggregation_number` | 2..200; en la SDF solo se dibujan 12 cabezas como máximo |
| `complementary_surfaces`, `van_der_waals`, `electrostatic` | conteos/fracciones adimensionales |
| `intensity_threshold` | desplaza la superficie (dilata/erosiona la SDF) |

## Funciones principales (20 públicas)
| Función | Devuelve | Descripción |
|---|---|---|
| `host_guest_binding_energy(superficies, H_bonds, VdW, elec)` | kJ/mol | −20·H − 5·VdW − 30·elec − 10·superficies |
| `association_constant(ΔG, T)` | K_a | exp(−ΔG·1000/(R·T)), techo e⁴⁰ |
| `recognition_selectivity(ΔG_target, ΔG_competidor)` | ratio | exp(−ΔΔG·1000/(R·298)) |
| `geometric_complementarity(cavidad, huésped, flexibilidad)` | 0..1.2 | 1 − |Δtamaño|/mayor, modulado por flexibilidad |
| `preorganization_energy(libre, unido)` | kJ/mol | E_unido − E_libre |
| `self_assembly_criterion(ΔH, ΔS, T)` | kJ/mol | ΔH − T·ΔS (espontáneo si < 0) |
| `self_assembly_rate(conc, K_a, E_a, T)` | tasa | K_a·e^(−E_a/RT)·conc² (segundo orden) |
| `assembled_structure_stability(enlaces, corrección, T)` | kJ/mol | 10·enlaces − 0.01·T·(1−corrección) |
| `cooperativity_factor(ΣE_ind, E_total)` | factor | E_total / ΣE_ind (>1 = cooperativo) |
| `defect_formation_rate(T, E_enlace, E_defecto)` | 0..1 | exp(−(E_d−E_b)/RT) acotado |
| `molecular_motor_efficiency(trabajo, entrada)` | 0..1 | trabajo/entrada acotado |
| `molecular_motor_rotation_speed(E_a, T, barrera)` | rev/s | e^(−(E_a+barrera)/RT)·10⁶ |
| `molecular_machine_work_cycle(salida, reset)` | kJ/mol | salida − reset |
| `molecular_switch_state(entrada, umbral, estado)` | 0/1 | conmuta si entrada ≥ umbral |
| `molecular_actuator_force(distancia, energía)` | fuerza | energía / distancia |
| `micelle_aggregation_number(cola, packing)` | 2..200 | cola²/packing acotado |
| `critical_micelle_concentration(cola, área, T)` | fracción molar | 0.01·e^(−0.5·cola)/(área)·e^(0.005(T−298)) |
| `liquid_crystal_phase(aspecto, conc, T)` | 0/1/2 | esméctico < 300 K, nemático 300–350 K, isótropo fuera |
| `vesicle_radius(bicapa, tensión, bending)` | longitud | sqrt(κ/σ), mínimo media bicapa |
| `molecular_capsule_stability(espesor, volumen, huésped)` | 0..10 | (V/huésped³)·espesor·10 acotado |

## Helpers internos (`*_field`)
Precalculan con `let` las direcciones y las distancias una sola vez, para que las SDF no repitan subexpresiones. No son API de usuario, pero son invocables: `sphere_field`, `torus_field`, `cylinder_x_field`, `cylinder_y_field`, `micelle_head_field`, `vesicle_shell_field`, `hex_tile_field`.

## SDFs definidas
| SDF | Descripción |
|---|---|
| `spherical_micelle_structure(N, longitud, umbral)` | Núcleo esférico + hasta 12 cabezas polares en direcciones esféricas fijas, ocultas por `1−step(N,i)` |
| `vesicle_liposome_structure(R, bicapa, umbral)` | Lámina esférica hueca centrada en el radio medio |
| `catenane_interlocked_rings(R, tubo, giro, umbral)` | Dos toros entrelazados; el segundo inclinado por `giro` |
| `rotaxane_structure(eje, R_anillo, tubo, tope, umbral)` | Eje X, anillo deslizante animado con `w`, dos topes esféricos |
| `host_receptor_cavitand(cavidad, funcionalidades, umbral)` | Pared hueca + huésped esférico + hasta 8 grupos de borde |
| `hexagonal_self_assembly(baldosa, interacción, defectos, umbral)` | Rejilla hexagonal 5×5 con defectos animados por `w` + campo de interacción |

## Corregido 2026-11-04
- **Reescritura completa (v1.1)**. La v1.0 no compilaba: usaba `>` (L119), arrays de posiciones, `for`, `if/else`, `union_array` y primitivas con vectores. Todas las funciones devuelven ahora escalares o códigos numéricos.
- **Dependencias reducidas a `std_math_core.hm`**: las v1.0 (`std_math_algebra`, `std_chem_organic`, `std_chem_coordination`, `std_bridge_biochem`) no se usaban o no existen como dependencia real.
- **API sin strings**: `liquid_crystal_phase` devuelve 0/1/2; `molecular_switch_state` devuelve 0/1; `rim_functionalities` es un conteo 0..8.
- **Optimización con `let`**: las 7 funciones `*_field` precalculan radios, ejes y coordenadas; las 6 SDF son una sola expresión `custom(...) − intensity_threshold`. La micela usa `min` encadenado en lugar de `union_array`; el ensamblaje hexagonal es `union(custom(tiles), custom(interacción))` binario.
- **Micela visual acotada a 12 cabezas** (antes el rango de `N` era 2..200): las cabezas se distribuyen con ángulo áureo y cada una se apaga con `1−step(N,i)`; una cabeza apagada tiene radio 0 y no contribuye al `min`.
- **Catenano**: el segundo anillo se inclina con `sin/cos` del ángulo de giro (antes usaba una matriz de rotación con arrays).
- **Rotaxano**: el anillo se desplaza con `sin(w·0.5)` a lo largo del eje (animación real, no posición fija).
- **Cavitando**: pared hueca `|r − 0.85·cavidad| − 0.15·cavidad`, huésped central y 8 grupos de borde sobre un anillo, apagados por `1−step(funcionalidades,i)`.
- **Hexagonal**: 25 baldosas en coordenadas hexagonales (desplazamiento de fila por paridad), defecto animado por `w`, sin `for`.

## Verificado 2026-11-04
- Compila: exit 0 (`libreria/std_chem_supramolecular.hm`, ~9 ms).
- Probe de 24 funciones: exit 0, **24/24 nodos**. Probe de 6 defs + variantes: exit 0, **8 customs** exactos, `.rih` sin `for`/`if`/`let`/`atan2`/`union_array`/`append`/`?` residuales.
- **24/24 valores de funciones OK** (`supra_checks.ps1`, evaluando la expresión emitida en `.rih` contra una segunda implementación en PowerShell): `binding_energy=−90`, `K_a(−90,298)=5.9601e15`, `selectividad(−90,−80)=56.5986`, `complementariedad(.5,.45,.5)=0.99`, `preorganización=−5`, `criterio=−79.8`, `tasa(.1,1000,20)=0.00312168`, `estabilidad(5,.8,300)=49.4`, `cooperatividad=1.2`, `defectos(298,−50,−40)=0.0176683`, `eficiencia=0.5`, `rotación(20,298,10)=5.51546`, `ciclo=5`, `switch(10,5,0)=1`, `switch(3,5,1)=1`, `actuador(0.1,2)=20`, `N(2,.5)=8`, `CMC(10,.5,298)=1.34759e−4`, `fase(3.5,.4,290)=2`, `fase(...320)=1`, `fase(...400)=0`, `fase(2.5,.25,290)=1`, `R_vesícula(.05,1,.5)=0.707106`, `cápsula(.02,1,.5)=1.6`.
- **17/17 puntos SDF OK** (evaluados con `y=0`): micela centro −0.04, cabeza0 −0.05, vesícula pared −0.025, vesícula centro +0.25, catenano anillos −0.02, rotaxano eje/anillo −0.02, cavitando pared −0.0225 / huésped −0.075 / borde −0.02, hex baldosa −0.01, hex defecto ≈0, hex interacción ≈0.001, micela N=1 cabeza0 −0.05, cabeza5 apagada 0.
- Nota de verificación: una cabeza **apagada** (radio 0) situada en su propio centro da SDF exactamente 0, que es el mínimo; no es un error de la SDF, es la definición de esfera de radio nulo.
- Sin artefactos residuales (probes y `.rih/.ont/.obs` eliminados).

## Notas
V1.1. La química supramolecular estudia ensamblajes moleculares complejos: reconocimiento = complementariedad + preorganización; autoensamblaje espontáneo si ΔG < 0; las máquinas moleculares convierten energía en trabajo. Catenanos y rotaxanos son entrelazados mecánicos (MEIs) de topología no trivial. Las SDF son representaciones visuales acotadas: la micela muestra como máximo 12 cabezas y el ensamblaje hexagonal 25 baldosas. El resolver sustituye parámetros textualmente; pasar expresiones complejas a una SDF que repite el parámetro multiplica el tamaño del `.rih`, así que conviene precalcular valores.
