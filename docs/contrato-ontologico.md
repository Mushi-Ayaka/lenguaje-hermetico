# Contrato Ontológico — Lenguaje Hermético

**Propósito:** Qué es correcto, qué falta, qué no pertenece.

---

## 1. Principio Fundamental

El universo es campo continuo en $\mathbb{R}^4$. No hay objetos. Hay puntos con estado:

$$\mathcal{C}(\mathbf{p}) = (d, m, \mathbf{c}, A)$$

El color visible se obtiene combinando estos campos por píxel.

---

## 2. Lo Que Existe Hoy

| Componente | Estado |
|---|---|
| Pipeline `.herm → RIH → render → .png` | Funcional |
| SDFs: sphere, box, cylinder, torus, cone, plane, capsule, rounded_box, octahedron | Hardcodeadas en C++ |
| Operadores: union, subtract, intersect, smooth_*, repeat, twist, elongate, displace | Hardcodeados en C++ |
| Materiales: definidos en la escena con `tensor` 1x8 | Catálogo legado `archived/libreria/materiales.hm` (archivado, DL-27) |
| Luces: directional + point con shadow | En C++ |
| `let`, `def`/`instance` parametrizadas | Funcional |
| Animación por eje W | Funcional |
| Modo volume (Beer-Lambert) | Funcional |

---

## 3. Lo Que Falta (simplificado)

No necesitamos 14 librerías ni formato `.ether` nuevo. Solo falta:

**En el evaluador:**
- `x, y, z` ligados al punto del rayo (hoy solo `w`)
- `hash()` determinista para formas naturales

**En el RIH y renderer:**
- Que las SDFs sean expresiones evaluables en vez de switch en C++
- Que las primitivas vivan en `.hm` como `def`s

**En el lenguaje:**
- Operadores de forma natural (erosión, fragmento, cristalización) como librerías `.hm`
- Composición volumétrica de materiales (múltiples substancias en un mismo cuerpo)
- Constructor `region` para jerarquía espacial (BVH ontológico)

---

## 4. Lo Que NO Pertenece al Lenguaje

| Feature | Razón |
|---------|-------|
| Mallas poligonales | Rompe SDF exacta |
| Post-processing (bloom, DOF) | Falsea la observación |
| Texturas/imágenes externas | Discontinuidad, dependencia externa |
| Ruido estadístico (noise, fbm) | No determinista |
| Redes neuronales | Aproximación, no exactitud |
| Control procedural (`if`/`for` como instrucciones) | El lenguaje es declarativo |
| Pipeline MLIR / GTE / PTR / CTB | Proyecto aparte en `Lienzo y renderizado/` — no parte del core |
| Formato `.rib` binario | Escritor sin lector — formato huérfano |
| 14 librerías ontológicas | Sobrediseño — solo necesitamos archivos `.hm` |

---

## 5. Prioridad Inmediata

1. `x, y, z` en el evaluador ~10 líneas
2. Expresiones SDF en RIH (reemplazar enum `sdf_type` + `params` por texto de expresión)
3. Renderer evalúa expresiones SDF por punto
4. Mover primitivas a `std_sdf.hm`
5. `hash()` en evaluador
6. Librerías `.hm` de dominio (std_optics, std_geology, std_biology)
