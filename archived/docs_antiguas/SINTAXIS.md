# Sintaxis del Lenguaje Hermético

## 1. Convenciones

- **Bilingüe**: todas las palabras clave tienen versión en español e inglés
- **Sensible a mayúsculas**: las palabras clave van en **minúsculas**
- **Comentarios**: `//` para una línea, `/* */` para bloque
- **Identificadores**: `[A-Za-z_][A-Za-z0-9_]*` — las cadenas se usan con `"comillas"`
- **Determinista**: toda función debe tener una ley analítica subyacente. No se permite ruido no acotado.

## 2. Tipos y Literales

| Tipo | Sintaxis | Ejemplos |
|------|----------|----------|
| Escalar | número con/sin decimal | `42`, `3.1416`, `-1.5` |
| Vector 2 | `[a, b]` | `[800, 600]` |
| Vector 3 | `[a, b, c]` | `[0.5, 0.3, 0.1]` |
| Vector 4 | `[a, b, c, d]` | `[1, 0, 0, 1]` |
| Cadena | `"texto"` | `"madera"` |
| Booleano | `true` / `false` | `true` |
| Ángulo | número + `deg` o `rad` | `45deg`, `1.5708rad` |

## 3. Estructura General

```
program      ::= scene_block (material_block | node_block)*
scene_block  ::= ("scene" | "escena") STRING? "{" scene_body "}"
scene_body   ::= axis_decl* camera_decl? background?
material_block ::= ("material") STRING "{" material_body "}"
node_block   ::= ("node" | "nodo") STRING? "{" node_body "}"
```

## 4. Bloque de Escena

```
scene "Mi Mundo" {
    // Ejes (resolución de salida y cortes temporales)
    axes { X: 1920, Y: 1080, W: 300 }

    // Cámara
    camera {
        position:   [0, 0, -5]
        target:     [0, 0, 0]
        up:         [0, 1, 0]
        fov:        60
        aperture:   0.0          // depth of field (0 = pinhole)
        focal_dist: 5.0
    }

    // Color de fondo
    background: [0.1, 0.1, 0.2]
}
```

`W` es el eje temporal (cuarta dimensión). La variable `t` es implícita y va de `0` a `W-1`.

Campos de `camera`:

| Campo | Español | Tipo | Default |
|-------|---------|------|---------|
| `position` | `posición` | `[x,y,z]` | `[0,0,0]` |
| `target` | `objetivo` | `[x,y,z]` | `[0,0,0]` |
| `up` | `arriba` | `[x,y,z]` | `[0,1,0]` |
| `fov` | `fov` | escalar (grados) | `60` |
| `aperture` | `apertura` | escalar | `0.0` |
| `focal_dist` | `distancia_focal` | escalar | `10.0` |

## 5. Materiales

```
material "madera" {
    base_color:     [0.55, 0.27, 0.07]
    roughness:      0.8
    metallic:       0.0
    emission:       [0, 0, 0]
    emission_mode:  isotropic       // isotropic | directional
    emission_dir:   [0, -1, 0]      // solo si emission_mode = directional
    ior:            1.5
    transmission:   0.0
    opacity:        1.0
    subsurface:     0.0
    blend_mode:     alpha           // alpha | add | replace
}
```

### Español / Inglés

| Inglés | Español | Tipo | Default |
|--------|---------|------|---------|
| `base_color` | `color_base` | `[r,g,b]` (0-1) | `[1,1,1]` |
| `roughness` | `rugosidad` | escalar (0-1) | `0.5` |
| `metallic` | `metálico` | escalar (0-1) | `0.0` |
| `emission` | `emisión` | `[r,g,b]` (0-1) | `[0,0,0]` |
| `emission_mode` | `modo_emisión` | `isotropic` / `directional` | `isotropic` |
| `emission_dir` | `dir_emisión` | `[x,y,z]` | `[0,-1,0]` |
| `ior` | `ior` | escalar | `1.5` |
| `transmission` | `transmisión` | escalar (0-1) | `0.0` |
| `opacity` | `opacidad` | escalar (0-1) | `1.0` |
| `subsurface` | `subsuperficie` | escalar (0-1) | `0.0` |
| `blend_mode` | `modo_mezcla` | `alpha` / `add` / `replace` | `alpha` |

## 6. Nodos y SDFs

### 6.1 Nodo Hoja (con SDF)

```
node "esfera" {
    mode: solid               // solid | volume
    sdf: sphere(1.0)
    material: "madera"
    position:  [0, 0, 0]
    rotation:  [0, 0, 0]     // en grados
    scale:     [1, 1, 1]
    visible:   true
    density_scale: 1.0        // solo si mode = volume
    blend_mode: alpha         // override del material
}
```

### 6.2 Nodo Grupo (con hijos)

```
node "silla" {
    position: [0, 0, 0]
    rotation: [0, 0, 0]
    scale:    [1, 1, 1]

    node "asiento" {
        sdf: box([1.0, 0.1, 0.8])
        material: "madera"
        position: [0, 0.5, 0]
    }

    node "respaldo" {
        sdf: box([0.8, 0.6, 0.08])
        material: "madera"
        position: [0, 0.9, -0.35]
        rotation: [5, 0, 0]
    }
}
```

Las transformaciones del padre se heredan (multiplican) a los hijos.

### 6.3 Operadores SDF

```
node "cajón" {
    sdf: subtract(box([1, 1, 1]), box([0.8, 0.8, 0.8]))
    material: "madera"
}

node "columna" {
    sdf: smooth_union(cylinder(0.3, 2.0), sphere(0.5), 0.2)
    material: "piedra"
}

node "decoración" {
    sdf: intersect(sphere(1.0), box([1.5, 1.5, 0.5]))
    material: "metal"
}
```

### 6.4 SDFs Primitivas

| Función | Parámetros | Descripción |
|---------|-----------|-------------|
| `sphere(r)` | radio | Esfera centrada en origen |
| `box(size)` | `[w,h,d]` | Caja centrada en origen |
| `cylinder(r, h)` | radio, altura | Cilindro en Y |
| `capsule(a, b, r)` | extremo A, extremo B, radio | Cápsula |
| `torus(r_mayor, r_menor)` | radios | Toro |
| `plane(normal, d)` | normal, distancia | Plano infinito |
| `cone(r, h)` | radio base, altura | Cono en Y |
| `rounded_box(size, r)` | tamaño, radio | Caja con bordes redondeados |
| `octahedron(r)` | radio | Octaedro |
| `tesseract(r)` | radio | Hipercubo 4D |
| `sphere4d(r)` | radio | Hiperesfera 4D |

### 6.5 Operadores SDF

| Función | Parámetros | Descripción |
|---------|-----------|-------------|
| `union(a, b)` | dos SDFs | Unión (min) |
| `subtract(a, b)` | dos SDFs | Resta (max(a, -b)) |
| `intersect(a, b)` | dos SDFs | Intersección (max) |
| `smooth_union(a, b, k)` | dos SDFs, factor k | Unión suave |
| `smooth_subtract(a, b, k)` | dos SDFs, factor k | Resta suave |
| `smooth_intersect(a, b, k)` | dos SDFs, factor k | Intersección suave |
| `repeat(a, spacing)` | SDF, espaciado | Repite infinitamente |
| `twist(a, angle)` | SDF, ángulo | Tuerce alrededor de Y |
| `elongate(a, size)` | SDF, tamaño | Estira el espacio |
| `displace(a, f)` | SDF, función analítica | Desplaza con función (ej. `sin(x*10)*cos(z*10)*0.1`) |

## 7. Animación (Dependencia Temporal)

El eje `W` (cuarta dimensión, tiempo) está disponible como variable implícita `t` en las expresiones. `t` va de `0` a `W-1`.

```
node "estrella" {
    sdf: sphere(0.3)
    material: "metal"
    position: [sin(t * 2) * 3, cos(t * 1.5) * 2, 0]
}

node "ojo" {
    sdf: sphere(0.5)
    material: "ojos"
    position: [0, sin(t) * 0.1 + 0.5, 0]
}
```

### 6.6 Variación Orgánica (sin ruido)

Para superficies orgánicas, se usa superposición de funciones analíticas en lugar de ruido:

```
node "montaña" {
    sdf: displace(plane([0, 1, 0], 0), sin(x * 5) * cos(z * 5) * 0.1 + sin(x * 13 + z * 7) * 0.05)
    material: "tierra"
}

node "duna" {
    sdf: displace(plane([0, 1, 0], 0), sin(x * 2 + z * 1.5) * 0.3 + cos(x * 4 - z * 3) * 0.15)
    material: "arena"
}
```

## 8. Herencia de Propiedades

Los hijos heredan transformaciones del padre. Las propiedades no definidas en un hijo se heredan del padre.

```
node "galería" {
    material: "pared"      // material por defecto para todos los hijos

    node "cuadro_1" {
        sdf: box([1, 0.8, 0.05])
        position: [-2, 1.5, 0]
    }

    node "cuadro_2" {
        sdf: box([1, 0.8, 0.05])
        material: "especial"    // override
        position: [0, 1.5, 0]
    }
}
```

## 9. Instancias (Reutilización)

```
def "pata" {
    sdf: cylinder(0.05, 0.8)
    material: "metal"
}

node "mesa" {
    position: [0, 0, 0]

    node "tablero" {
        sdf: box([1.2, 0.05, 0.8])
        material: "madera"
        position: [0, 0.8, 0]
    }

    instance "pata" { position: [-0.5, 0.4, -0.3] }
    instance "pata" { position: [0.5, 0.4, -0.3] }
    instance "pata" { position: [-0.5, 0.4, 0.3] }
    instance "pata" { position: [0.5, 0.4, 0.3] }
}
```

## 10. Expresiones

Soporta expresiones aritméticas en posiciones, rotaciones, escalas, y parámetros SDF:

| Operador | Símbolo | Ejemplo |
|----------|---------|---------|
| Suma | `+` | `a + b` |
| Resta | `-` | `a - b` |
| Multiplicación | `*` | `a * b` |
| División | `/` | `a / b` |
| Potencia | `^` | `a ^ 2` |
| Negación | `-` | `-a` |

### Funciones Incorporadas

Todas las funciones son deterministas y analíticas. No se incluye ruido estadístico (viola la métrica Lipschitz y el determinismo causal). Para variación orgánica, usar superposición de funciones trigonométricas (ej. `sin(x*freq1)*cos(z*freq2)*amp`).

| Función | Descripción |
|---------|-------------|
| `sin(x)`, `cos(x)`, `tan(x)` | Trigonométricas |
| `abs(x)` | Valor absoluto |
| `sqrt(x)` | Raíz cuadrada |
| `min(a, b)` | Mínimo |
| `max(a, b)` | Máximo |
| `clamp(x, lo, hi)` | Limitar |
| `lerp(a, b, t)` | Interpolación lineal |
| `length(v)` | Magnitud de vector 3D |
| `length4(v)` | Magnitud de vector 4D |
| `dot(a, b)` | Producto punto 3D |
| `dot4(a, b)` | Producto punto 4D |
| `cross(a, b)` | Producto cruz 3D |
| `normalize(v)` | Vector unitario 3D |
| `normalize4(v)` | Vector unitario 4D |
| `smoothstep(lo, hi, x)` | Transición suave |
| `mix(a, b, t)` | Mezcla (sinónimo de lerp) |

## 11. Ejemplo Completo

```
scene "Bodegón" {
    axes { X: 800, Y: 600, W: 1 }

    camera {
        position: [0, 1.5, 5]
        target:   [0, 1, 0]
        fov:      50
    }

    background: [0.05, 0.05, 0.1]
}

material "madera_oscura" {
    base_color: [0.4, 0.2, 0.05]
    roughness:  0.9
    metallic:   0.0
}

material "cobre" {
    base_color: [0.72, 0.45, 0.2]
    roughness:  0.3
    metallic:   1.0
}

material "vidrio" {
    base_color:  [0.9, 0.95, 1.0]
    roughness:   0.0
    metallic:    0.0
    ior:         1.5
    transmission: 0.9
    opacity:     0.3
}

node "mesa" {
    position: [0, 0, 0]

    node "tablero" {
        sdf: box([2.0, 0.1, 1.2])
        material: "madera_oscura"
        position: [0, 0.8, 0]
    }

    node "pata_1"  { sdf: cylinder(0.04, 0.8); material: "madera_oscura"; position: [-0.8, 0.4, -0.5] }
    node "pata_2"  { sdf: cylinder(0.04, 0.8); material: "madera_oscura"; position: [0.8, 0.4, -0.5] }
    node "pata_3"  { sdf: cylinder(0.04, 0.8); material: "madera_oscura"; position: [-0.8, 0.4, 0.5] }
    node "pata_4"  { sdf: cylinder(0.04, 0.8); material: "madera_oscura"; position: [0.8, 0.4, 0.5] }
}

node "jarron" {
    sdf: smooth_union(cylinder(0.3, 0.6), sphere(0.35), 0.1)
    material: "cobre"
    position: [-0.5, 1.1, 0]
}

node "esfera" {
    sdf: sphere(0.3)
    material: "vidrio"
    position: [0.5, 1.1, 0]
}
```

## 12. Animación: Ejemplo con Bola Saltarina

```
scene "Salto" {
    axes { X: 400, Y: 400, W: 120 }
    camera {
        position: [0, 1, 6]
        target: [0, 1, 0]
        fov: 50
    }
    background: [0.1, 0.15, 0.3]
}

material "goma" {
    base_color: [1, 0.2, 0.2]
    roughness: 0.7
    metallic: 0.0
}

node "suelo" {
    sdf: plane([0, 1, 0], 0)
    material: "goma"
}

node "bola" {
    sdf: sphere(0.3)
    material: "goma"
    position: [0, abs(sin(t * 3)) * 2 + 0.3, 0]
}
```
