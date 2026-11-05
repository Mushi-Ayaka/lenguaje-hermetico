# std_chem_theoretical.hm

**Capa:** Química (4.7 — Química Teórica y Computacional)
**Versión:** 1.1
**Fuente:** "Computational Chemistry" by Jensen
**Dependencias:** std_math_core.hm
**Fecha:** 6 de junio de 2026 (reescrita 2026-11-04)

## Cómo
Implementa métodos de química computacional: funcionales DFT (B3LYP híbrido), correlación MP2, costos computacionales (HF O(N⁴), DFT O(N³), CCSD(T) O(N⁷), ahorro por Resolution of Identity), dinámica molecular (Velocity-Verlet, Lennard-Jones, enlaces y ángulos armónicos, torsión, par de Ewald), Monte Carlo (Metropolis, traslación, rotación, media de Gibbs), optimización geométrica (gradiente numérico, Hessiano diagonal, convergencia, BFGS 1D), análisis vibracional (frecuencias con signo, intensidad IR, actividad Raman, energía de punto cero) y SDFs de visualización (PES, trayectoria MD, histograma MC, rampa de optimización, modo vibracional animado, malla DFT).

El lenguaje no tiene arrays, `for`, `if`, callbacks ni retorno de vectores: **todo se resuelve componente a componente**. Las funciones que en la v1.0 recorrían listas ahora reciben ya calculados los escalares que necesitan (energías, integrales, diferencias) y devuelven un único número.

## Por qué
La química computacional es el laboratorio virtual donde se simulan moléculas. Esta librería conecta la teoría química con métodos numéricos prácticos.

## Para qué
- Estimar costos computacionales de métodos cuánticos (HF, DFT, CCSD(T), RI)
- Evaluar funcionales DFT y correlación MP2 de forma escalar
- Integrar dinámica molecular con Velocity-Verlet y campos de fuerza
- Muestrear configuraciones con Monte Carlo (Metropolis, Gibbs)
- Optimizar geometrías con gradientes, Hessianos y BFGS
- Calcular frecuencias vibracionales, IR, Raman y energía de punto cero
- Visualizar PES, trayectorias, histogramas, modos normales y mallas DFT

## Convenciones numéricas
| Parámetro | Valores |
|---|---|
| `density`, `gradient`, `kinetic_energy_density` | densidades del funcional (u.a.) |
| `energy_gap`, `integral` | brecha orbital (u.a.) e integral de dos electrones |
| `basis_functions`, `grid_points`, `auxiliary_functions` | conteos enteros ≥ 1 |
| `position_value`, `velocity`, `acceleration`, `dt` | una componente del vector, unidades coherentes |
| `distance_r`, `r0`, `sigma` | longitud |
| `epsilon`, `barrier`, `k_bond`, `k_angle` | energía (kJ/mol o u.a. según el modelo) |
| `theta`, `theta0`, `phi`, `gamma` | radianes |
| `alpha_value`, `cutoff` | parámetro de Ewald y radio de corte |
| `energy_change`, `energy_a`, `energy_b` | **J por partícula** (metropolis/gibbs usan k_B) |
| `temperature` | kelvin |
| `uniform_sample` | número en [0,1) que aporta el llamador (no hay `random()`) |
| `wavenumber` | cm⁻¹ (zero_point_energy lo convierte a kJ/mol) |
| `atom_count` | 0..5 (la SDF dibuja como máximo 5 átomos) |
| `grid_density` | 0..5 (malla 5×5 de puntos DFT) |
| `intensity_threshold` | desplaza la superficie (dilata/erosiona la SDF) |

## Funciones principales (27 públicas)
| Función | Devuelve | Descripción |
|---|---|---|
| `b3lyp_exchange_correlation(ρ, ∇ρ, τ)` | energía | 0.20·τ + 0.80·ρ^(4/3) + 0.72·|∇ρ|²/ρ^(4/3) + 0.81·ρ·e^(−0.1ρ) |
| `mp2_pair_correlation(gap, integral)` | energía | −0.25·integral²/(gap+ε) |
| `mp2_energy_computation(gap, integral, pares)` | energía | pares × correlación media de par |
| `hartree_fock_cost(N)` | operaciones | N⁴ |
| `dft_cost(N, G)` | operaciones | N³ + N·G |
| `ccsdt_cost(N)` | operaciones | N⁷ |
| `ri_approximation_savings(N, M)` | 0..1 | 1 − M/N² (fracción ahorrada) |
| `velocity_verlet_position(x, v, a, dt)` | longitud | x + v·dt + ½·a·dt² |
| `velocity_verlet_velocity(v, a, dt)` | velocidad | v + ½·a·dt (media velocidad) |
| `lennard_jones_energy(r, ε, σ)` | energía | 4ε[(σ/r)¹² − (σ/r)⁶] |
| `lennard_jones_force(r, ε, σ)` | fuerza | 24ε/r·[2(σ/r)¹² − (σ/r)⁶] |
| `harmonic_bond_energy(r, r0, k)` | energía | ½·k·(r−r0)² |
| `harmonic_angle_energy(θ, θ0, k)` | energía | ½·k·(θ−θ0)² |
| `torsion_energy(φ, V, n, γ)` | energía | V·(1 + cos(nφ − γ)) |
| `ewald_direct_pair(qi, qj, r, α, corte)` | energía | qiqj·erfc(αr)/r dentro del corte; 0 fuera |
| `metropolis_criterion(ΔE, T)` | 0..1 | min(1, e^(−ΔE/(k_BT))); el llamador compara con un aleatorio |
| `mc_translation_step(Δmax, u)` | longitud | (u−0.5)·2·Δmax |
| `mc_rotation_step(θmax, u)` | radianes | (u−0.5)·2·θmax |
| `gibbs_sampling_average(a, b, Ea, Eb, T)` | valor | (a·e^(−Ea/kT) + b·e^(−Eb/kT))/(Σ pesos) |
| `energy_gradient_numerical(E+, E−, δ)` | derivada | (E+ − E−)/(2δ); el llamador evalúa las energías |
| `hessian_diagonal(E++, E+−, E−+, E−−, δ)` | derivada 2ª | (E++ − E+− − E−+ + E−−)/(4δ²) |
| `optimization_converged(‖g‖, ΔE, tol_g, tol_E)` | 0/1 | 1 si ambos criterios se cumplen |
| `bfgs_hessian_update_1d(Δg, Δx)` | curvatura | Δg/Δx (condición secante 1D) |
| `vibrational_frequency(λ, masa)` | frecuencia | signo(λ)·sqrt(|λ|/masa); negativa = imaginaria |
| `infrared_intensity(∂μ/∂Q)` | intensidad | (∂μ/∂Q)² |
| `raman_activity(∂α/∂Q)` | actividad | (∂α/∂Q)² |
| `zero_point_energy(ν̃)` | kJ/mol | ½·h·c·ν̃·N_A (ν̃ en cm⁻¹) |

## Helpers internos
Precalculan con `let` y evitan repetir subexpresiones en las SDF. No son API de usuario, pero son invocables: `sphere_field`, `erfc_approx`, `pes_field`, `md_trajectory_field`, `mc_bars_field`, `opt_pathway_field`, `vib_atom_field`, `dft_grid_dot_field`.

`erfc_approx` usa la aproximación racional de Numerical Recipes (precisión ~1×10⁻⁷) y aplica `erfc(−x) = 2 − erfc(x)`; no existe `erfc` nativa en el lenguaje.

## SDFs definidas
| SDF | Descripción |
|---|---|
| `potential_energy_surface_computational(barrera, separación, umbral)` | Doble pozo: V = barrera·min(((x²−a²)/a²)², 1) + 0.1·z² |
| `md_trajectory_visualization(amplitud, λ, tubo, deriva, umbral)` | Tubo 3D alrededor de y = deriva·x + A·sin(2πx/λ + w/2) |
| `monte_carlo_sampling_visualization(aceptación, altura, ancho, umbral)` | Barras periódicas de altura variable + esfera de aceptación |
| `geometry_optimization_pathway(inicio, fin, ondulación, umbral)` | Rampa descendente con ondulación amortiguada y pared en z |
| `vibrational_mode_animation(amplitud, frecuencia, n, umbral)` | Hasta 5 átomos oscilando en y con fase alterna (animado por `w`) |
| `dft_integration_grid_visualization(densidad, umbral)` | Núcleo central + malla 5×5 de puntos con peso gaussiano, recortada por densidad |

## Corregido 2026-11-04
- **Reescritura completa (v1.1)**. La v1.0 no compilaba: usaba `<` en L129, arrays, `for`, `if/else`, `len`, `append`, `union_array`, `random()`, `erfc`, `distance`, `rotate_vector`, `dot_product`, `matrix_vector_product`, `diagonal_approximation` y retornos de vectores. Todo eso desaparece.
- **Dependencias reducidas a `std_math_core.hm`**: la v1.0 listaba módulos que no aportaban nada o no existen.
- **MP2**: los cuatro bucles anidados se sustituyen por la correlación de un par representativo por el número de pares activos (`mp2_pair_correlation` + `mp2_energy_computation`). Documentado como aproximación escalar.
- **Ewald**: la suma sobre pares se sustituye por `ewald_direct_pair`, con `erfc_approx` (aproximación racional) y compuerta de corte `1−step(corte, r)`.
- **Metropolis** devuelve la **probabilidad** de aceptación (0..1) en vez de un booleano con `random()`; `mc_translation_step`/`mc_rotation_step` reciben el número uniforme del llamador.
- **Gibbs** pasa de media aritmética de configuraciones a **media ponderada por Boltzmann** de dos configuraciones.
- **Gradiente/Hessiano**: el llamador evalúa la energía en los puntos desplazados; la librería solo aplica la diferencia central. Sin callbacks.
- **BFGS**: la actualización matricial se reduce a la curvatura secante 1D `Δg/Δx`.
- **Frecuencias vibracionales** conservan el signo (negativo = frecuencia imaginaria); la v1.0 solo devolvía la magnitud y perdía la información de punto de silla.
- **IR y Raman**: se elimina el factor `normal_mode²` que la v1.0 multiplicaba arbitrariamente; las fórmulas quedan I ∝ |∂μ/∂Q|² y A ∝ |∂α/∂Q|².
- **Añadido** `zero_point_energy` (ν̃ en cm⁻¹ → kJ/mol) y `lennard_jones_energy` (la v1.0 solo tenía la fuerza).
- **SDFs**: las seis dependían de listas de geometrías o de grids con bucles. Ahora son campos analíticos con `mod_unit`/`floor`/`sin` y `min` encadenado (5 átomos, 5×5 puntos), animados con `w`.
- **Pase de optimización**: exponentes fijos por productos encadenados (N⁴, N⁷, (σ/r)⁶, (σ/r)¹², cuadrados); `pow` solo queda en B3LYP con exponente 4/3. Helpers `*_field` con `let` para no repetir raíces, fases ni pesos. Constantes muertas (`GAS_CONSTANT_THEOR`, `HARTREE_TO_KJ`) eliminadas; parámetro `z` muerto fuera de `mc_bars_field`.

## Verificado 2026-11-04
- Compila: exit 0 (`libreria/std_chem_theoretical.hm`, ~1 ms).
- Probe de 30 funciones: exit 0, **30 nodos**. Probe de 6 defs: exit 0, **6 customs** exactos, `.rih` sin `for`/`if`/`let`/`atan2`/`union_array`/`append`/`?` residuales.
- **30/30 valores de funciones OK** (`theor_checks.ps1`, evaluando la expresión emitida en `.rih` contra una implementación independiente en PowerShell): `b3lyp(.5,.1,.3)=0.780871`, `mp2_pair(.5,.2)=−0.01999996`, `mp2_total=×4=−0.0799998`, `HF(10)=10000`, `DFT(10,1000)=11000`, `CCSDT(10)=1e7`, `RI(10,50)=0.5`, `verlet_pos(1,.5,2,.1)=1.06`, `verlet_vel(.5,2,.1)=0.6`, `LJ_energy(1.2,1,1)=−0.890963`, `LJ_force(1.2,1,1)=−2.2117`, `bond(1.2,1,100)=2`, `angle(1.6,π/2,50)=0.0213214`, `torsión(.5,2,3,.1)=2.33993`, `Ewald(1,−1,1,.5,2)=−0.4795`, `Ewald fuera=0`, `Metropolis(4.141947e−21,300)=0.367879`, `traslación(.1,.75)=0.05`, `rotación(.5,.2)=−0.3`, `Gibbs(1,2,0,kT,300)=1.26894`, `gradiente(1.1,.9,.01)=10`, `Hessiano diag=50`, `convergencia sí=1`, `no=0`, `BFGS(.2,.1)=2`, `ν(4,2)=1.41421`, `ν(−4,2)=−1.41421`, `IR(.3)=0.09`, `Raman(.2)=0.04`, `ZPE(3000)=17.944`.
- **15/15 puntos SDF OK** (evaluados con `w=0`): PES centro −0.5, mínimo x=0.8 → 0, x=1 → −0.158203; MD x=0 → −0.02, cresta (0.2,0.11) → −0.02; MC esfera −0.05, barra x=0.1 → −0.14; ruta x=0 → −0.35, x=1.5 → −0.105256; vibración átomo0/2 −0.03, átomo3 apagado ≈0; DFT núcleo −0.04, punto (0.12) −0.0143907, punto (0.24, apagado) 0.
- **2/2 comprobaciones de animación** con `w=1`: átomo0 vibra a y=−0.0227041 (SDF −0.03) y la cresta MD sube a y=0.0479424 (SDF −0.02).
- Sin artefactos residuales (probes y `.rih/.ont/.obs` eliminados).

## Notas
V1.1. El funcional B3LYP es una aproximación híbrida con coeficientes canónicos (0.20 HF + 0.80 Slater + 0.72 B88 + 0.81 LYP). Los métodos están escalarizados: no hay vectores ni callbacks, así que el llamador aporta energías, integrales y números aleatorios ya calculados. Las SDF son visualizaciones acotadas de conceptos abstractos (PES, trayectorias, histogramas, modos normales, mallas): la vibración dibuja como máximo 5 átomos y la malla DFT es 5×5. El resolver sustituye parámetros textualmente; pasar expresiones complejas a una SDF que repite el parámetro multiplica el tamaño del `.rih`, así que conviene precalcular valores.
