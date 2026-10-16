# std_math_algebra.hm

**Capa:** 1.2 — Álgebra Abstracta y Geometría Diferencial
**Versión:** 0.2
**Fuente:** Álgebra lineal, vectores, matrices, formas bilineales
**Dependencias:** std_math_core.hm
**Fecha:** 5 de junio de 2026

## Cómo

`std_math_algebra.hm` extiende las operaciones geométricas básicas de `std_math_core.hm` hacia álgebra vectorial completa y geometría diferencial. Implementa operaciones para vectores 2D, 3D y 4D (producto punto, norma, normalización, proyección, reflexión, rotación), rotaciones 3D por ejes (Euler) y fórmula de Rodrigues, álgebra polinomial (evaluación por Horner, discriminante, raíces cuadráticas), geometría diferencial (curvatura 2D, normal de Frenet, gradiente y laplaciano numéricos), formas bilineales y cuadráticas (forma cuadrática diagonal, elipsoide implícito), y análisis tensorial básico (traza, determinante 3x3, norma de Frobenius). También define SDFs geométricas extendidas: elipsoide, superelipsoide, hiperboloide y paraboloide.

## Por qué

Cubre las operaciones de álgebra lineal que el hardware gráfico maneja en paralelo pero que Hermético debe expresar algebraicamente. Sin vectores no hay rotación, sin rotación no hay orientación espacial, y sin orientación no es posible posicionar formas en escenas 3D. La geometría diferencial provee curvatura y normales esenciales para iluminación y deformación de SDFs.

## Para qué

Se usa para transformar, rotar, reflejar y proyectar formas en escenas `.herm`. Las rotaciones por Euler y Rodrigues permiten animar objetos en 3D. El álgebra polinomial facilita curvas paramétricas y superficies alabeadas. Las SDFs definidas (elipsoide, superelipsoide, hiperboloide, paraboloide) ofrecen formas canónicas listas para usar en escenas.

## Funciones principales

| Nombre | Descripción breve |
|---|---|
| `v2_dot(ax, ay, bx, by)` | Producto punto 2D |
| `v2_cross(ax, ay, bx, by)` | Producto cruz 2D (escalar) |
| `v2_len(ax, ay)` | Norma euclidiana 2D |
| `v2_len_sq(ax, ay)` | Norma al cuadrado 2D |
| `v2_nx(ax, ay)` / `v2_ny(ax, ay)` | Normalización 2D por componente |
| `v2_angle(ax, ay)` | Ángulo polar del vector |
| `v2_proj_len(ax, ay, bx, by)` | Longitud de proyección de a sobre b |
| `v2_reflect_{x,y}(vx, vy, nx, ny)` | Reflexión 2D de v sobre normal n |
| `rot2{x,y}(px, py, ang)` | Rotación 2D |
| `v3_dot(ax, ay, az, bx, by, bz)` | Producto punto 3D |
| `v3_len(ax, ay, az)` | Norma euclidiana 3D |
| `v3_len_sq(ax, ay, az)` | Norma al cuadrado 3D |
| `v3_n{x,y,z}(ax, ay, az)` | Normalización 3D por componente |
| `v3_cross_{x,y,z}(ax,...,bz)` | Producto vectorial 3D por componente |
| `v3_reflect_{x,y,z}(vx,...,nz)` | Reflexión 3D de v sobre normal n |
| `v3_proj_scalar(vx,...,nz)` | Proyección escalar de v sobre n |
| `v3_lerp_{x,y,z}(a, b, t)` | Interpolación lineal vectorial |
| `v4_dot(ax,...,bw)` | Producto punto 4D |
| `v4_len(ax, ay, az, aw)` | Norma euclidiana 4D |
| `v4_len_sq(ax, ay, az, aw)` | Norma al cuadrado 4D |
| `v4_n{x,y,z,w}(ax,...,aw)` | Normalización 4D por componente |
| `rotX_{x,y,z}(px, py, pz, ang)` | Rotación 3D alrededor de eje X |
| `rotY_{x,y,z}(px, py, pz, ang)` | Rotación 3D alrededor de eje Y |
| `rotZ_{x,y,z}(px, py, pz, ang)` | Rotación 3D alrededor de eje Z |
| `rodrigues_{x,y,z}(vx,...,ang)` | Rotación 3D (fórmula de Rodrigues) |
| `poly2(x, a0, a1, a2)` | Polinomio de grado 2 (Horner) |
| `poly3(x, a0, a1, a2, a3)` | Polinomio de grado 3 (Horner) |
| `poly4(x, a0, a1, a2, a3, a4)` | Polinomio de grado 4 (Horner) |
| `discriminant(a, b, c)` | Discriminante cuadrático: b² − 4ac |
| `quadratic_root1(a, b, c)` | Primera raíz cuadrática |
| `quadratic_root2(a, b, c)` | Segunda raíz cuadrática |
| `curvature2d(yp, ypp)` | Curvatura de curva plana y(x) |
| `frenet_nx(dydx)` / `frenet_ny(dydx)` | Normal de Frenet 2D |
| `grad_x(fx+, fx−, h)` | Gradiente numérico (dif. finitas, x) |
| `grad_y(fy+, fy−, h)` | Gradiente numérico (dif. finitas, y) |
| `laplacian(f0, fx+, fx−, fy+, fy−, h)` | Laplaciano numérico 2D |
| `quad_form_diag(vx, vy, vz, a11, a22, a33)` | Forma cuadrática diagonal |
| `ellipsoid_sdf(px, py, pz, a, b, c)` | Elipsoide implícito: x²/a² + y²/b² + z²/c² − 1 |
| `trace3(a11, a22, a33)` | Traza de matriz 3×3 diagonal |
| `det3(a..j)` | Determinante de matriz 3×3 |
| `frobenius_diag(a11, a22, a33)` | Norma de Frobenius de matriz diagonal |

## SDFs definidas

| Nombre | Descripción |
|---|---|
| `ellipsoid(semi_a, semi_b, semi_c)` | SDF exacta de elipsoide (Quilez) |
| `superellipsoid(semi_a, semi_b, semi_c, exp_n)` | Superelipsoide genérico: |x/a|ⁿ + ... = 1 |
| `hyperboloid1(semi_a, semi_b, semi_c)` | Hiperboloide de 1 hoja: x²/a² + z²/c² − y²/b² = 1 |
| `paraboloid(r_param, height)` | Paraboloide: z = (x² + z²) / (2r²) |

## Notas

- Las rotaciones 3D por Euler separan cada componente (x, y, z) en funciones individuales porque Hermético no tiene tipos vectoriales.
- La fórmula de Rodrigues permite rotar alrededor de un eje arbitrario, no solo los canónicos X, Y, Z.
- El elipsoide SDF usa la aproximación bound de Inigo Quilez.
