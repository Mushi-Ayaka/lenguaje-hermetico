# Implementación

## 1. Compilador `herm`

### 1.1 Fase de Parse (`herm_lexer.cpp`, `herm_parser.cpp`)

El lexer tokeniza el archivo .herm en una secuencia de tokens. El parser construye un AST donde cada nodo representa una declaración del lenguaje.

**Estructuras internas (`contrato/herm.h`):**
- `HermScene`: escena con cámara, ejes, fondo
- `HermMaterial`: material con propiedades PBR
- `HermNode`: nodo del grafo de escena (SDF, group, instance, def)
- `HermSdfOp`: operación SDF (primitiva o compuesta)
- `HermExpr`: expresión analítica (árbol de operaciones)

### 1.2 Fase de Resolución (`herm_resolver.cpp`)

El resolvedor:
1. Resuelve imports (busca en directorio local y `libreria/`)
2. Resuelve herencia de materiales (ascenso en jerarquía)
3. Construye grafo de nodos planos con transformaciones efectivas
4. Evalúa expresiones animadas para cada fotograma

**Herencia de transformaciones:**
```
composeTransform(T_local, T_parent):
    translate_out = T_parent.translate + T_local.translate
    rotate_out    = T_parent.rotate + T_local.rotate
    scale_out     = T_parent.scale * T_local.scale
```

### 1.3 Fase de Generación RIH (`herm_rih.cpp`)

Genera la RIH en JSON (o binario `.rib` con `--bin`):
- Asigna IDs a materiales, nodos, definiciones
- Serializa árbol de SDFs
- Escribe transformaciones con expresiones animadas

### 1.4 Evaluador de Expresiones (`herm_eval.cpp`)

Parser recursivo descendente para expresiones matemáticas:

**Soporte:**
- Operadores: `+`, `-`, `*`, `/`, `^`
- Funciones: `sin`, `cos`, `tan`, `abs`, `sqrt`, `min`, `max`, `clamp`, `lerp`, `mix`, `length`, `length4`, `dot`, `dot4`, `cross`, `normalize`, `normalize4`, `smoothstep`
- Variables: `w`, `t` (fotograma actual), `x`, `y`, `z` (en displace)
- Constantes numéricas

**Evaluación:**
```
eval_expr(expr, vars):
    if number: return number
    if variable: return vars[name]
    if binary_op: return apply_op(op, eval_expr(a), eval_expr(b))
    if function: return apply_fn(fn, eval_exprs(args))
```

### 1.5 Defs Parametrizadas (Sustitución en AST)

Cuando un `def` declara parámetros y un `instance` suministra argumentos, el resolvedor no puede pre-resolver el `def` a un nodo RIH (porque los parámetros son desconocidos en tiempo de definición). En su lugar:

1. El `def` parametrizado se almacena en un mapa separado (`ast_def_map`).
2. Al encontrar un `instance` con argumentos, el resolvedor clona el árbol SDF del `def`.
3. Recorre recursivamente cada `ExprNode` en el árbol y sustituye los nombres de parámetros por el texto de los argumentos.
4. El nodo resultante se resuelve como un nodo SDF ordinario (no como INSTANCE), con las transformaciones del `def` + las del `instance` compuestas.

**Ejemplo:**
```
// def "pata"(h, r) { sdf: cylinder(r, h); position: [0, h/2, 0]; }
// instance "pata"(0.8, 0.05)
// → después de sustitución: sdf: cylinder(0.05, 0.8); position: [0, 0.8/2, 0];
```

### 1.6 Renderer Directo (`herm_render.cpp`)

Usa `stb_image_write.h` para escribir PNG. Soporta dos modos:

- **Sólido**: sphere tracing clásico, normal por gradiente numérico, sombra por shadow ray, shading Blinn-Phong + PBR simplificado.
- **Volumen**: integración Beer-Lambert a lo largo del rayo. Acumula densidad y emisión por pasos fijos. Compone con el color del sólido subyacente: `color = vol_accumulado + transmitancia * color_solido`.

```
rayMarchVol(ro, rd, t_min, t_max, steps):
    density_acum = 0
    color_acum = [0,0,0]
    step = (t_max - t_min) / steps
    for i in 0..steps:
        p = ro + rd * t
        res = evalScene(p)
        if res.vol_density > 0:
            local_density = res.vol_density * step
            transmittance = exp(-local_density)
            absorbed = 1 - transmittance
            color_acum += absorbed * vol_emission * (1 - density_acum)
            density_acum += local_density
            if density_acum > 0.99: break
        t += step
    return color_acum, exp(-density_acum)
```

- La densidad se obtiene de la distancia negativa del SDF: `density = max(0, -d) * 0.1`.
- El color del volumen se toma de `emission` del material (o `base_color * 0.5` si no hay emisión).
- En `renderRih`, si hay volumen, se blend: `color = vol_color + transmitancia * color_solido`.
- Si no hay hit sólido pero hay volumen, se muestra solo el volumen.

## 2. Backend `operador_tb`

### 2.1 Lector JSON (`json_reader.cpp`)

Parser JSON minimalista recursivo descendente que lee RIH:
- Soporta objetos, arrays, strings, números, booleanos, null
- Navega por el árbol JSON para extraer escena, materiales, nodos

### 2.2 Lector RIH (`rih_reader.cpp`)

Convierte JSON → estructuras `herm::Rih`:
- `RihScene`: ejes, cámara, fondo
- `RihMaterial[]`: tabla de materiales
- `RihDef[]`: definiciones
- `RihNode[]`: nodos (sdf, group, instance)

### 2.3 Renderer (`renderer.cpp`)

**Sphere Tracing:**
```
sphere_trace(ray_origin, ray_dir):
    t = 0
    for step in 0..MAX_STEPS:
        p = ray_origin + ray_dir * t
        d = evalSdfTree(p, grafo_nodos)
        if abs(d) < EPS: return t  // hit
        t += d
        if t > MAX_DIST: return -1  // miss
```

**Sombra:**
```
shadowRay(hit_pos, light_dir):
    t = SHADOW_BIAS
    for step in 0..MAX_STEPS:
        p = hit_pos + light_dir * t
        d = evalSdfTree(p, grafo_nodos)
        if d < EPS: return true  // bloqueado
        t += d
        if t > MAX_DIST: return false  // sin sombra
```

**Normal (gradiente numérico):**
```
normal(p):
    eps = 1e-4
    dx = [eps, 0, 0]
    dy = [0, eps, 0]
    dz = [0, 0, eps]
    return normalize([
        evalSdf(p+dx) - evalSdf(p-dx),
        evalSdf(p+dy) - evalSdf(p-dy),
        evalSdf(p+dz) - evalSdf(p-dz)
    ])
```

**Shading:**
```
shade(p, n, v, material, light):
    // Ambiente
    color = ambient * material.base_color

    // Difuso (Lambert)
    color += diffuse * material.base_color * max(0, dot(n, l))

    // Especular (Blinn-Phong)
    h = normalize(l + v)
    color += specular * pow(max(0, dot(n, h)), shininess)

    // Metálico (PBR simplificado)
    color += metallic * fresnel_schlick(...)
```

### 2.4 Escritor TB (`tb_writer.cpp`)

Escribe buffer RGBA en formato .tb:
- Divide imagen en bloques 8×8
- Detecta bloques de color sólido → escribe P0
- Comprime bloques RAW si es necesario
- Termina con BLOCK_END

## 3. SDFs (Evaluación de Árbol)

Los parámetros de primitivas SDF (`SdfNode::params`) son tipo `Expr[4]`, no `f32[4]`. Cada parámetro puede ser una constante (`is_expr=false`) o una expresión (`is_expr=true`) que se evalúa por fotograma en `evaluateFrame()` vía `evalNodeSdfParams()`. En tiempo de render, el valor ya está en `params[i].constant`.

`evalSdfTree()` evalúa recursivamente el árbol SDF:

```
evalSdfTree(p, sdf_node):
    switch sdf_node.type:
        case SPHERE:    return length(p) - sdf_node.radius
        case BOX:       return sdBox(p, sdf_node.size)
        case CYLINDER:  return sdCylinder(p, sdf_node.radius, sdf_node.height)
        case TORUS:     return sdTorus(p, sdf_node.radii)
        case PLANE:     return dot(p, sdf_node.normal) + sdf_node.d
        case CONE:      return sdCone(p, sdf_node.radius, sdf_node.height)
        case CAPSULE:   return sdCapsule(p, sdf_node.a, sdf_node.b, sdf_node.radius)
        case ROUNDED_BOX: return sdRoundedBox(p, sdf_node.size, sdf_node.radius)
        case OCTAHEDRON: return sdOctahedron(p, sdf_node.radius)
        case TESSERACT:  return sdTesseract(p, sdf_node.radius)
        case SPHERE4D:   return sdSphere4D(p, sdf_node.radius, sdf_node.w_offset)
        case UNION:     return min(evalSdfTree(p, a), evalSdfTree(p, b))
        case SUBTRACT:  return max(evalSdfTree(p, a), -evalSdfTree(p, b))
        case INTERSECT: return max(evalSdfTree(p, a), evalSdfTree(p, b))
        case SMOOTH_UNION: return smoothMin(evalSdfTree(p, a), evalSdfTree(p, b), k)
        case REPEAT:    return evalSdfTree(repeatSpace(p, sdf_node.spacing), a)
        case TWIST:     return evalSdfTree(twistSpace(p, sdf_node.angle), a)
        case ELONGATE:  return evalSdfTree(elongateSpace(p, sdf_node.size), a)
        case DISPLACE:  return evalSdfTree(p, a) + evalExpr(sdf_node.f, p)
```

## 4. Expresiones en Transformaciones (Group Composition)

### 4.1 Acumulación de transformaciones

```
accumulate(node, grafo):
    acc = { translate: [0,0,0], rotate: [0,0,0], scale: [1,1,1] }
    current = node
    while current has parent in group chain:
        parent = grafo[current.parent]
        if parent.type == GROUP:
            acc.translate += parent.transform.translate
            acc.rotate    += parent.transform.rotate
            acc.scale     *= parent.transform.scale
        current = parent
    return acc
```

### 4.2 Composición de instancias

```
composeInstance(def_node, instance_node):
    return {
        translate: def_node.translate + instance_node.translate,
        rotate:    def_node.rotate + instance_node.rotate,
        scale:     def_node.scale * instance_node.scale
    }
```

## 5. Plataforma y Compilación

- **Compilador**: Visual Studio 2022 (MSVC v19.44)
- **Estándar**: C++17
- **Flags**: `/O2` (optimizado), `/EHsc` (excepciones C++), `/std:c++17`
- **Build**: `build.bat` en cada subproyecto

```
cl /EHsc /std:c++17 /O2 /Fo"build/" /Fe"build/herm.exe" *.cpp /link
```

## 6. Dependencias Externas

| Dependencia | Uso | Integración |
|-------------|-----|-------------|
| `stb_image_write.h` | Escritura PNG en `--render` | 3rdparty, copia directa |
| `render.exe` | Visualización .tb | Herramienta externa existente |
