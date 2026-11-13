# Guía de Uso del Lenguaje Hermético

## 1. Estructura de un archivo .herm

```
scene "Nombre" {
    axes { X: ancho, Y: alto, W: fotogramas }
    camera {
        position: [x, y, z];
        target:   [x, y, z];
        up:       [x, y, z];
        fov:      grados;
    }
    background: [r, g, b];
}

import "libreria.hm"

let PI = 3.14159265;

material "nombre" { ... }

light "sol" { ... }

def "nombre" { ... }

node "nombre" { ... }

instance "nombre" { ... }
```

## 2. Bloque de Escena

### 2.1 Axes

Define resolución de salida y duración de la animación:

```
axes { X: 800, Y: 600, W: 30 }
```

- `X`: ancho en píxeles
- `Y`: alto en píxeles
- `W`: número de fotogramas (1 = imagen fija)

### 2.2 Cámara

```
camera {
    position: [0, 2, 5];
    target:   [0, 0, 0];
    up:       [0, 1, 0];
    fov:      50;
}
```

| Campo | Tipo | Default | Descripción |
|-------|------|---------|-------------|
| `position` | `[x,y,z]` | `[0,0,0]` | Posición de la cámara |
| `target` | `[x,y,z]` | `[0,0,0]` | Punto al que mira |
| `up` | `[x,y,z]` | `[0,1,0]` | Vector arriba |
| `fov` | escalar | 60 | Campo de visión en grados |

### 2.3 Background

```
background: [0.05, 0.05, 0.1];
```

Color RGB del fondo (valores 0-1).

## 3. Materiales

### 3.1 Definición

```
material "madera" {
    base_color:     [0.55, 0.27, 0.07];
    roughness:      0.8;
    metallic:       0.0;
    emission:       [0, 0, 0];
    emission_mode:  isotropic;
    ior:            1.5;
    transmission:   0.0;
    opacity:        1.0;
    subsurface:     0.0;
    blend_mode:     alpha;
}
```

### 3.2 Campos

| Inglés | Español | Tipo | Default |
|--------|---------|------|---------|
| `base_color` | `color_base` | `[r,g,b]` 0-1 | `[1,1,1]` |
| `roughness` | `rugosidad` | 0-1 | 0.5 |
| `metallic` | `metalico` | 0-1 | 0.0 |
| `emission` | `emision` | `[r,g,b]` ≥0 | `[0,0,0]` |
| `emission_mode` | `modo_emision` | `isotropic`/`directional` | `isotropic` |
| `emission_dir` | `dir_emision` | `[x,y,z]` | `[0,-1,0]` |
| `ior` | `ior` | ≥1 | 1.5 |
| `transmission` | `transmision` | 0-1 | 0.0 |
| `opacity` | `opacidad` | 0-1 | 1.0 |
| `subsurface` | `subsuperficie` | 0-1 | 0.0 |
| `blend_mode` | `modo_mezcla` | `alpha`/`add`/`replace` | `alpha` |

## 4. Nodos

### 4.1 Nodo hoja con SDF

```
node "esfera" {
    mode: solid;
    sdf: sphere(1.0);
    material: "madera";
    position: [0, 0, 0];
    rotation: [0, 0, 0];
    scale:    [1, 1, 1];
}
```

### 4.2 Nodo grupo

```
node "silla" {
    position: [0, 0, 0];

    node "asiento" {
        sdf: box(1.0, 0.1, 0.8);
        material: "madera";
        position: [0, 0.5, 0];
    }
    node "respaldo" {
        sdf: box(0.8, 0.6, 0.08);
        material: "madera";
        position: [0, 0.9, -0.35];
        rotation: [5, 0, 0];
    }
}
```

Los hijos heredan las transformaciones del padre (traslación aditiva, rotación aditiva, escala multiplicativa).

## 5. SDFs Primitivas

Los parámetros de primitivas SDF aceptan **expresiones analíticas completas**, no solo constantes:

```
// Constantes
sdf: sphere(1.0);
sdf: cylinder(0.05, 0.8);

// Expresiones aritméticas
sdf: sphere(1.5 * 0.5);           // = 0.75, evaluado en compilación
sdf: cylinder(radio * 0.5, alto); // con defs parametrizadas

// Expresiones animadas (por fotograma)
sdf: sphere(0.65 + sin(w * 2) * 0.35);  // radio pulsa con w
sdf: box(w * 0.1, 0.5, 0.5);            // ancho crece con w
```

Las expresiones en SDFs siguen las mismas reglas que las expresiones en transformaciones (ver §9).

| Sintaxis | Descripción |
|----------|-------------|
| `sphere(r)` | Esfera de radio r |
| `box(w, h, d)` | Caja de ancho w, alto h, fondo d |
| `cylinder(r, h)` | Cilindro en Y de radio r y altura h |
| `capsule(ay, r)` | Cápsula en Y de altura ay y radio r |
| `torus(r_mayor, r_menor)` | Toro |
| `plane(nx, ny, nz, d)` | Plano infinito con normal (nx,ny,nz) y distancia d |
| `cone(r, h)` | Cono en Y de radio r y altura h |
| `rounded_box(w, h, d, r)` | Caja con bordes redondeados de radio r |
| `octahedron(r)` | Octaedro de radio r |
| `tesseract(r)` | Hipercubo 4D (corte transversal = caja redondeada) |
| `sphere4d(r)` | Hiperesfera 4D (radio aparente se reduce con w-offset) |

## 6. Operadores SDF

| Sintaxis | Descripción |
|----------|-------------|
| `union(a, b)` | Unión booleana |
| `subtract(a, b)` | Resta booleana |
| `intersect(a, b)` | Intersección booleana |
| `smooth_union(a, b, k)` | Unión suave con factor k |
| `smooth_subtract(a, b, k)` | Resta suave |
| `smooth_intersect(a, b, k)` | Intersección suave |
| `repeat(a, s)` | Repetición con espaciado s = [sx, sy, sz] |
| `twist(a, angle)` | Torsión alrededor de Y proporcional a y |
| `elongate(a, [hx, hy, hz])` | Estira el SDF (resta half-extents) |
| `displace(a, expr)` | Desplaza con expresión analítica (ej. `sin(x*10)*0.1`) |

### 6.1 Ejemplos

```
sdf: subtract(box(1, 1, 1), box(0.8, 0.8, 0.8));
sdf: smooth_union(cylinder(0.3, 2.0), sphere(0.5), 0.2);
sdf: displace(plane(0, 1, 0, 0), sin(x*5)*cos(z*5)*0.1);
```

## 7. Instancias (Reutilización)

### 7.1 Instancias simples

```
def "pata" {
    sdf: cylinder(0.05, 0.8);
    material: "metal";
}

node "mesa" {
    instance "pata" { position: [-0.5, 0.4, -0.3]; }
    instance "pata" { position: [ 0.5, 0.4, -0.3]; }
    instance "pata" { position: [-0.5, 0.4,  0.3]; }
    instance "pata" { position: [ 0.5, 0.4,  0.3]; }
}
```

La instancia hereda el SDF y material de la definición, pero puede sobrescribir transformaciones.

### 7.2 Instancias parametrizadas

`def` puede declarar parámetros que se sustituyen en el cuerpo:

```
def "pata"(h, r) {
    sdf: cylinder(r, h);
    material: "metal";
}

node "mesa" {
    instance "pata"(0.8, 0.05) { position: [-0.5, 0.4, -0.3]; }
    instance "pata"(0.8, 0.05) { position: [ 0.5, 0.4, -0.3]; }
}
```

Los parámetros se sustituyen textualmente en las expresiones del SDF y las transformaciones. Las expresiones con `w`/`t` permanecen activas para animación por fotograma.

## 8. Animación (Eje W)

La variable `w` (o `t` como alias) representa el fotograma actual (0 a W-1).

### 8.1 Posición animada

```
node "estrella" {
    sdf: sphere(0.3);
    material: "metal";
    position: [sin(w * 2) * 3, cos(w * 1.5) * 2, 0];
}
```

### 8.2 Rotación animada

```
node "rueda" {
    sdf: cylinder(0.5, 0.2);
    material: "caucho";
    rotation: [0, w * 30, 0];
}
```

### 8.3 Expresiones en instancias

```
instance "tecla" { position: [0, 0.2 - max(0, 1 - abs(w - 12)) * 0.12, 0]; }
```

### 8.4 Reutilización de pulsos

```
// Pulso triangular: sube en frame N, baja en frame N+2
max(0, 1 - abs(w - N))
```

## 9. Expresiones

### 9.1 Operadores aritméticos

| Símbolo | Operación |
|---------|-----------|
| `+` | Suma |
| `-` | Resta / Negación |
| `*` | Multiplicación |
| `/` | División |
| `^` | Potencia |

### 9.2 Funciones incorporadas

| Función | Descripción |
|---------|-------------|
| `sin(x)` | Seno |
| `cos(x)` | Coseno |
| `tan(x)` | Tangente |
| `abs(x)` | Valor absoluto |
| `sqrt(x)` | Raíz cuadrada |
| `min(a, b)` | Mínimo |
| `max(a, b)` | Máximo |
| `clamp(x, lo, hi)` | Limitar |
| `lerp(a, b, t)` | Interpolación lineal |
| `mix(a, b, t)` | Sinónimo de lerp |
| `length(v)` | Magnitud de vector 3D |
| `length4(v)` | Magnitud de vector 4D |
| `dot(a, b)` | Producto punto 3D |
| `dot4(a, b)` | Producto punto 4D |
| `cross(a, b)` | Producto cruz 3D |
| `normalize(v)` | Vector unitario 3D |
| `normalize4(v)` | Vector unitario 4D |
| `smoothstep(lo, hi, x)` | Transición suave |

## 10. Herencia de Propiedades

- Las transformaciones se heredan del padre al hijo
- Traslación y rotación: se **suman** a las del padre
- Escala: se **multiplica** por la del padre
- Material: si no se define en un hijo, hereda del ancestro más cercano

```
node "galeria" {
    material: "pared";

    node "cuadro" {
        sdf: box([1, 0.8, 0.05]);
        material: "especial";    // override
        position: [0, 1.5, 0];
    }
}
```

## 11. Imports

```
import "std_math_core.hm"
```

Busca el archivo en:
1. El directorio del archivo .herm actual
2. El directorio `libreria/`

## 12. Modo de objetos

| Modo | Descripción | Render |
|------|-------------|--------|
| `solid` | Superficie dura | A = 1 dentro, A = 0 fuera |
| `volume` | Medio participativo | Densidad continua, luz se atenúa |

## 13. Herramienta CLI (`herm.exe`)

```
herm <archivo.herm> [opciones]

Opciones:
  -o <archivo>    Archivo de salida
  --bin           Salida binaria (.rib)
  --tb            Emitir .tb directo
  --gte           Emitir .gte
  --render        Pipeline completo hasta .png
  --frames N-M    Cortes en W (defecto: 0..W-1 si W>1)

Ejemplos:
  herm escena.herm
  herm escena.herm -o salida.rih
  herm escena.herm --frames 0-29
  herm escena.herm --render -o salida.png
  herm escena.herm --render --frames 0-29 -o fotograma_
```

`--render` y `--gte` pliegan constantes y expresiones temporales (`w`/`t`). Si un parámetro SDF o una transformación depende de `x`/`y`/`z` (o de variables de sombreado), fallan con error y no escriben PNG/GTE: usa `--ont` para el camino de bytecode.

## 14. Ejemplo completo: Bodegón

```
scene "Bodegon" {
    axes { X: 800, Y: 600, W: 1 }
    camera {
        position: [0, 1.5, 5];
        target:   [0, 1, 0];
        fov:      50;
    }
    background: [0.05, 0.05, 0.1];
}

material "madera_oscura" {
    base_color: [0.4, 0.2, 0.05];
    roughness:  0.9;
    metallic:   0.0;
}

material "cobre" {
    base_color: [0.72, 0.45, 0.2];
    roughness:  0.3;
    metallic:   1.0;
}

node "mesa" {
    node "tablero" {
        sdf: box([2.0, 0.1, 1.2]);
        material: "madera_oscura";
        position: [0, 0.8, 0];
    }
    node "pata_1" { sdf: cylinder(0.04, 0.8); material: "madera_oscura"; position: [-0.8, 0.4, -0.5]; }
    node "pata_2" { sdf: cylinder(0.04, 0.8); material: "madera_oscura"; position: [ 0.8, 0.4, -0.5]; }
    node "pata_3" { sdf: cylinder(0.04, 0.8); material: "madera_oscura"; position: [-0.8, 0.4,  0.5]; }
    node "pata_4" { sdf: cylinder(0.04, 0.8); material: "madera_oscura"; position: [ 0.8, 0.4,  0.5]; }
}

node "jarron" {
    sdf: smooth_union(cylinder(0.3, 0.6), sphere(0.35), 0.1);
    material: "cobre";
    position: [-0.5, 1.1, 0];
}

## 14. Variables `let`

### 14.1 Sintaxis

```
let PI = 3.14159265;
let TAU = pi * 2;
let RADIO = 1.5;
let ESCALA = RADIO * 0.5;
```

Las variables `let` se definen en el ámbito global del archivo (antes o después de scene, materiales, nodos). La palabra clave `let` se puede usar en español como `sea`.

```
sea NOMBRE = valor;
```

### 14.2 Comportamiento

- **Expansión en compilación**: `let` se resuelve textualmente durante la resolución. No llega al RIH — es como si hubieras escrito el valor directamente.
- **Cadenas**: `let A = B + 1; let B = 2;` funciona — el resolvedor itera hasta resolver todas las dependencias.
- **Límites de palabra**: `RADIO` no afecta a `RADIOLOGIA` ni a `radio`.
- **Built-ins protegidos**: `pi`, `e`, `w`, `t`, `x`, `y`, `z` son variables del evaluador y no pueden ser reemplazadas por `let`.

### 14.3 Usos típicos

**Constantes físicas:**
```
let PI = 3.14159265;
let TAU = PI * 2;
let E = 2.71828;
```

**Parámetros de escena:**
```
let ESCALA_GLOBAL = 1.0;
let ALTURA_MESA = 0.8;
let RADIO_PATA = 0.04;
```

**Combinado con defs parametrizadas:**
```
let ESCALA = 1.0;

def "tornillo"(h, r) {
    sdf: cylinder(r * ESCALA, h);
    material: "metal";
}
```

**Combinado con luces:**
```
let BRILLO_SOL = 0.7;
let BRILLO_RELLENO = 0.3;

light "sol" { type: directional; direction: [0.5, 0.8, 0.3]; intensity: BRILLO_SOL; }
lumbre "relleno" { type: point; position: [-2, 1, 2]; intensity: BRILLO_RELLENO; }
```

## 15. Luces

### 14.1 Sintaxis

```
light "sol" {
    type: directional;
    direction: [0.3, 0.8, 0.5];
    color: [1, 1, 1];
    intensity: 1.0;
}

lumbre "foco" {
    type: point;
    position: [2, 3, 0];
    color: [1, 0.5, 0.1];
    intensity: 0.8;
    falloff: 0.1;
}
```

### 14.2 Campos

| Inglés | Español | Tipo | Default | Descripción |
|--------|---------|------|---------|-------------|
| `type` | `tipo` | `directional` / `point` | `directional` | Tipo de luz |
| `direction` | `direccion` | `[x,y,z]` | `[0.3,0.8,0.5]` | Dirección (solo directional) |
| `position` | `posicion` | `[x,y,z]` | `[0,0,0]` | Posición (solo point) |
| `color` | `color_base` | `[r,g,b]` | `[1,1,1]` | Color de la luz |
| `intensity` | `intensidad` | escalar | 1.0 | Intensidad multiplicativa |
| `falloff` | `caida` | escalar ≥0 | 0.0 | Atenuación $1/(1+f·d^2)$ (solo point) |

### 14.3 Comportamiento

- **Directional**: la luz llega desde una dirección fija (como el sol). Sombra ray activo — los objetos proyectan sombras.
- **Point**: la luz emite desde un punto en todas direcciones. Atenuación por distancia si `falloff > 0`. Sin sombra ray por ahora.
- Si no se declaran luces, se usa una luz direccional blanca por defecto desde `normalize(0.3, 0.8, 0.5)`.
- Múltiples luces se acumulan: el color final es la suma de contribuciones, cada una con su propio shadow (direccionales) y atenuación (puntuales).

### 14.4 Ejemplo

```
scene "Luces" {
    axes { X: 400, Y: 300, W: 1 }
    camera { position: [0, 2, 5]; target: [0, 0, 0]; fov: 50; }
    background: [0.05, 0.05, 0.1];
}

light "sol" {
    type: directional;
    direction: [0.5, 0.8, 0.3];
    color: [1, 0.95, 0.8];
    intensity: 0.7;
}

lumbre "azul" {
    type: point;
    position: [-2, 1, 2];
    color: [0.2, 0.4, 1];
    intensity: 0.5;
    falloff: 0.05;
}

material "metal" {
    base_color: [0.8, 0.6, 0.3];
    metallic: 1.0;
    roughness: 0.2;
}

node "esfera" {
    sdf: sphere(1.0);
    material: "metal";
    position: [0, 0, 0];
}
```
```
