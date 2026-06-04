# Deuda Técnica — Fundamentos Pendientes

## 1. Diagnóstico

El pipeline compila `.herm` → `.png` y funciona. Pero hay una brecha entre **lo que implementamos en C++** y **lo que debería ser expresable en el lenguaje mismo**.

Problema raíz: las primitivas SDF (`sphere`, `box`, `cylinder`...), los modos de shading, los operadores booleanos están hardcodeados en el renderer como `switch (sdf_type)` en C++, cuando deberían ser `def`s en archivos `.hm` que el usuario puede ver, modificar y extender.

## 2. Deudas Identificadas

### 2.1 PBR como parche C++

**ESTADO: RESUELTO (v0.14)**

**Lo que hicimos:** Reemplazamos las propiedades hardcodeadas del Material PBR estático (base_color, metallic, roughness, etc.) en C++ por una matriz abstracta, el **Plano Tensorial 1x8** `[x, y, z, w, r, g, b, a]`.
**Por qué se resolvió:** El compilador y el runtime ya no tratan con conocimiento de dominio aislado de manera pre-fabricada. Ahora exponen una interfaz evaluable básica. Las funciones de óptica, materiales PBR, BRDF y similares se construirán como puras matemáticas y funciones orgánicas que vivirán directamente en los archivos `.hm` interactuando con los componentes tensores.

### 2.2 BVH como parche C++

**Lo que hicimos:** planeamos un árbol de bounding boxes en el RIH + traversal en el renderer.

**Por qué es deuda:** BVH es la expresión de que el espacio tiene jerarquía. Un árbol tiene tronco → ramas → hojas. Una ciudad tiene distritos → manzanas → edificios. El lenguaje debería anidar con `region` y que el BVH *emerja* de esa estructura, no ser un árbol invisible.

### 2.3 Sin x,y,z en el evaluador

**Lo que falta:** El evaluador (`herm_eval.cpp`) solo conoce `w, t, pi, e`. No conoce `x, y, z` del punto actual del rayo.

**Por qué es deuda:** Sin `x, y, z`, una `def "sphere"(r) { ... }` no puede evaluar `length(p) - r` en cada punto. Las SDFs tienen que estar hardcodeadas en C++ porque el evaluador no tiene el contexto 3D del punto que el renderer está evaluando.

**Un cambio desbloquea todo:** si el evaluador liga `x, y, z` al punto actual del rayo, entonces cualquier `def` puede ser una SDF. El `switch (sdf_type)` en C++ se vuelve obsoleto — las primitivas se mudan a `std_sdf.hm`.

### 2.4 Sin hash() determinista

**Lo que falta:** `hash(x, y, z, seed) → float` en [0,1], determinista, pura (como `sin` o `cos`).

**Por qué es deuda:** Sin hash no hay forma de expresar imperfección natural sin recurrir a `sin`/`cos` (que son suaves y periódicas — no pueden producir fractura, rugosidad no-uniforme, ni variación a múltiples escalas).

### 2.5 Materiales homogéneos

Hoy `material "oro"` aplica a todo el volumen del nodo. No se puede expresar "esto es 85% oro con 15% inclusiones de cuarzo y una costra de óxido en la superficie". Falta composición volumétrica.

### 2.6 Sin operadores de forma natural

Solo existen `sphere`, `box`, `cylinder`, `torus`, `cone`. No hay `fragmento`, `erosión`, `cristalización`, `estratificación`. Estos deben ser `def`s en librerías `.hm`, no C++.

## 3. Simplificación: lo que realmente falta

No necesitamos un formato `.ether` nuevo. El sistema de `import` + `def` ya existe. Solo falta **un cambio en el evaluador** y **que el renderer evalúe expresiones como SDFs**.

```
Estado actual:
  C++: switch(sdf_type) { case SPHERE: return length(p) - r; ... }
  .hm: nada — las primitivas no son expresables

Estado deseado:
  C++: eval(expr, {x, y, z, w, ...}) → distancia
  std_sdf.hm: def "sphere"(r) { length(p) - r }
  std_sdf.hm: def "box"(w, h, d) { length(max(abs(p) - vec3(w/2, h/2, d/2), 0)) }
```

No hay nuevos formatos. No hay nuevo parser. Solo:
1. `x, y, z` en el evaluador (ámbito dinámico, como `w`)
2. Expresiones SDF como texto en el RIH (en vez de enum `u8 sdf_type`)
3. El renderer evalúa la expresión con `p=(x,y,z)` en cada paso
4. Las primitivas actuales se mueven a `std_sdf.hm`

## 4. Roadmap corregido

1. Agregar `x, y, z` al evaluador (`herm_eval.cpp`, ~10 líneas)
2. Cambiar RIH: `SdfNode::expr_text` en vez de `sdf_type` + `params[4]`
3. El renderer evalúa la expresión SDF en cada paso del ray march
4. Mover primitivas actuales a `std_sdf.hm` (archivo `.hm`, sin formato nuevo)
5. Agregar `hash()` al evaluador
6. std_geology.hm, std_optics.hm, etc. como archivos `.hm` — conocimiento de dominio, no C++
7. BVH emerge de anidamiento `region` — constructor ontológico, no árbol invisible
