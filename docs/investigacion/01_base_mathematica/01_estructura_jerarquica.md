# JERARQUÍA DETALLADA - CAPA 1: BASE MATEMÁTICA
**Para el Lenguaje Hermético** - Enfoque en aplicaciones gráficas y simulaciones

## 1. LÓGICA MATEMÁTICA (1.1)

### 1.1.1 Lógica Proposicional y de Primer Orden
- **Propósito**: Validación de expresiones, condiciones booleanas
- **Aplicaciones**: Condicionales en SDFs, reglas de composición
- **Ecuaciones clave**: Operadores lógicos (∧, ∨, ¬, →, ↔)
- **Traducción Hermética**: `min()`, `max()`, `step()`, `smoothstep()`

### 1.1.2 Teoría de Conjuntos
- **Propósito**: Operaciones sobre campos espaciales
- **Aplicaciones**: Uniones, intersecciones, diferencias de SDFs
- **Ecuaciones clave**: ∪, ∩, \, ⊆, ∈
- **Traducción Hermética**: Operadores SDF existentes (`union()`, `intersect()`, `subtract()`)

### 1.1.3 Lógica Difusa
- **Propósito**: Transiciones suaves, mezclas volumétricas
- **Aplicaciones**: `smooth_union()`, mezclas de materiales
- **Ecuaciones clave**: Funciones de pertenencia μ(x), operadores T-norm/T-conorm
- **Traducción Hermética**: `smoothstep()`, interpolaciones

## 2. ÁLGEBRA ABSTRACTA Y GEOMETRÍA DIFERENCIAL (1.2)

### 2.1 ÁLGEBRA LINEAL (CRÍTICO)
#### 2.1.1 Espacios Vectoriales ℝ³, ℝ⁴
- **Propósito**: Operaciones sobre [x,y,z,w]
- **Aplicaciones**: Transformaciones, normales, iluminación
- **Operaciones clave**: 
  - Suma/resta vectorial
  - Producto escalar (dot product) → `dot()`
  - Producto vectorial (cross product) → `cross()`
  - Norma (magnitud) → `length()`
  - Normalización → `normalize()`

#### 2.1.2 Matrices y Transformaciones
- **Propósito**: Rotaciones, escalados, traslaciones
- **Aplicaciones**: Jerarquías de transformación en nodos
- **Operaciones clave**: 
  - Matrices 3×3, 4×4 (homogéneas)
  - Composición de transformaciones
  - Interpolación esférica (SLERP) para rotaciones

#### 2.1.3 Cuaterniones
- **Propósito**: Rotaciones sin gimbal lock
- **Aplicaciones**: Animación suave de rotaciones
- **Ecuaciones clave**: q = w + xi + yj + zk

### 2.2 GEOMETRÍA DIFERENCIAL (CRÍTICO)
#### 2.2.1 Campos Escalares y Vectoriales
- **Propósito**: SDFs son campos escalares Φ: ℝ³ → ℝ
- **Aplicaciones**: Evaluación de distancias, normales
- **Conceptos clave**: 
  - Gradiente ∇Φ → cálculo de normales
  - Derivada direccional D_vΦ

#### 2.2.2 Curvatura y Formas Diferenciales
- **Propósito**: Iluminación realista (BRDF basado en curvatura)
- **Aplicaciones**: Rugosidad microsuperficial
- **Conceptos clave**:
  - Primera forma fundamental (métrica)
  - Segunda forma fundamental (curvatura)
  - Curvatura media y gaussiana

#### 2.2.3 Variedades y Embeddings
- **Propósito**: Deformaciones de espacio-tiempo
- **Aplicaciones**: `twist()`, `displace()`, relatividad
- **Conceptos clave**: Mapeos suaves, coordenadas locales

### 2.3 ÁLGEBRA TENSORIAL (NUEVO - REQUERIDO)
#### 2.3.1 Tensores de Rango 1 (Vectores)
- **Propósito**: Coordenadas [x,y,z,w,r,g,b,a]
- **Aplicaciones**: Separación estricta geometría/color

#### 2.3.2 Tensores de Rango 2 (Matrices)
- **Propósito**: Transformaciones, métricas
- **Aplicaciones**: Deformación espacio-temporal

#### 2.3.3 Producto Tensorial
- **Propósito**: Composiciones de campos
- **Aplicaciones**: Campos vectoriales de color

## 3. CÁLCULO AVANZADO Y ECUACIONES DIFERENCIALES (1.3)

### 3.1 ANÁLISIS VECTORIAL (CRÍTICO)
#### 3.1.1 Gradiente, Divergencia, Rotacional
- **Propósito**: Operadores diferenciales en campos
- **Aplicaciones**: 
  - Gradiente → normales de superficie
  - Divergencia → campos vectoriales (fluidos)
  - Rotacional → vorticidad, campos magnéticos

#### 3.1.2 Teoremas Integrales
- **Propósito**: Conservación, flujo
- **Aplicaciones**: Volumen rendering (integral de línea)
- **Teoremas clave**: Gauss, Stokes, Green

### 3.2 ECUACIONES DIFERENCIALES
#### 3.2.1 EDOs (Ecuaciones Diferenciales Ordinarias)
- **Propósito**: Movimiento, evolución temporal
- **Aplicaciones**: Animación por eje W
- **Ejemplos**:
  - Movimiento armónico: x'' + ω²x = 0
  - Decaimiento exponencial: dx/dt = -kx

#### 3.2.2 EDPs (Ecuaciones Diferenciales Parciales)
- **Propósito**: Campos continuos, propagación
- **Aplicaciones**: 
  - Ecuación de calor: ∂u/∂t = α∇²u → difusión de color
  - Ecuación de onda: ∂²u/∂t² = c²∇²u → propagación
  - Ecuación de Laplace: ∇²u = 0 → campos armónicos

### 3.3 CÁLCULO VARIACIONAL
- **Propósito**: Optimización de formas, mínima energía
- **Aplicaciones**: Superficies mínimas (soap films), geodesias
- **Conceptos clave**: Principio de mínima acción, ecuaciones de Euler-Lagrange

## 4. TEORÍA DE LA PROBABILIDAD Y ESTADÍSTICA (1.4)

### 4.1 DISTRIBUCIONES DE PROBABILIDAD
#### 4.1.1 Distribuciones Continuas
- **Propósito**: Ruido, variación natural
- **Aplicaciones**:
  - Normal/Gaussiana → ruido gaussiano
  - Uniforme → aleatoriedad base
  - Exponencial → decaimientos

#### 4.1.2 Procesos Estocásticos
- **Propósito**: Patrones naturales, texturas
- **Aplicaciones**:
  - Proceso de Poisson → distribución de partículas
  - Caminos aleatorios (random walks) → difusión

### 4.2 ESTADÍSTICA ESPACIAL
#### 4.2.1 Campos Aleatorios
- **Propósito**: Texturas procedurales, terrenos
- **Aplicaciones**: Ruido de Perlin, Simplex, Worley
- **Conceptos clave**: Autocorrelación, espectro de potencia

#### 4.2.2 Geoestadística
- **Propósito**: Distribución espacial de propiedades
- **Aplicaciones**: Minerales en rocas, vegetación en biomas
- **Conceptos clave**: Kriging, variogramas

### 4.3 INFERENCIA BAYESIANA
- **Propósito**: Fusión de información, incertidumbre
- **Aplicaciones**: Mezcla de materiales con incertidumbre
- **Conceptos clave**: Teorema de Bayes, actualización de creencias

## 5. TEORÍA DE LA INFORMACIÓN Y SISTEMAS DINÁMICOS (1.5)

### 5.1 TEORÍA DE LA INFORMACIÓN
#### 5.1.1 Entropía y Información Mutua
- **Propósito**: Complejidad de patrones, compresión
- **Aplicaciones**: Medida de complejidad en texturas
- **Conceptos clave**: Entropía de Shannon H(X), información mutua I(X;Y)

#### 5.1.2 Codificación y Compresión
- **Propósito**: Representación eficiente de campos
- **Aplicaciones**: Compresión de SDFs jerárquicas

### 5.2 SISTEMAS DINÁMICOS
#### 5.2.1 Sistemas Continuos
- **Propósito**: Evolución temporal determinista
- **Aplicaciones**: Animación física (sin estado mutable)
- **Conceptos clave**: Espacio de fases, atractores, bifurcaciones

#### 5.2.2 Caos y Fractales
- **Propósito**: Patrones complejos, autosimilitud
- **Aplicaciones**: Terrenos fractales, nubes, vegetación
- **Conceptos clave**: 
  - Exponente de Lyapunov
  - Dimensión fractal
  - Sistemas iterados (IFS)

#### 5.2.3 Sistemas de Lindenmayer (L-Systems)
- **Propósito**: Crecimiento procedural, morfogénesis
- **Aplicaciones**: Plantas, árboles, patrones biológicos
- **Conceptos clave**: Gramáticas de reescritura, recursión

## 6. MATEMÁTICAS DISCRETAS (AGREGADO - REQUERIDO)

### 6.1 TEORÍA DE GRAFOS
- **Propósito**: Conectividad, redes
- **Aplicaciones**: Redes neuronales (patrones), estructuras cristalinas
- **Conceptos clave**: Vértices, aristas, caminos, ciclos

### 6.2 TEORÍA DE NÚMEROS
- **Propósito**: Patrones discretos, periodicidad
- **Aplicaciones**: `repeat()`, patrones de teselación
- **Conceptos clave**: Aritmética modular, números primos

### 6.3 COMBINATORIA
- **Propósito**: Conteo, disposiciones
- **Aplicaciones**: Distribuciones de instancias, variaciones

## 7. PRIORIDADES PARA IMPLEMENTACIÓN INICIAL

### ALTA PRIORIDAD (Implementar primero)
1. **Álgebra Vectorial 3D/4D** (2.1.1) - Crítico para todo
2. **Gradiente Numérico** (2.2.1) - Para normales de superficie
3. **Interpolaciones** (1.1.3) - Para transiciones suaves
4. **Ruido Procedural** (4.2.1) - Para texturas naturales

### MEDIA PRIORIDAD
5. **Transformaciones Matriciales** (2.1.2) - Para jerarquías
6. **EDOs Básicas** (3.2.1) - Para animación
7. **Distribuciones Probabilísticas** (4.1.1) - Para variación

### BAJA PRIORIDAD (posterior)
8. **Geometría Diferencial Avanzada** (2.2.2)
9. **EDPs** (3.2.2) - Para campos continuos
10. **Sistemas Dinámicos Complejos** (5.2)

## 8. RESTRICCIONES ESPECÍFICAS PARA LENGUAJE HERMÉTICO

### 8.1 SIN ESTADO MUTABLE
- Todas las funciones deben ser puras: output = f(input)
- No se puede almacenar estado entre evaluaciones
- La evolución temporal debe depender exclusivamente de `w`

### 8.2 EVALUACIÓN EN O(1)
- Las funciones deben evaluarse en tiempo constante
- No se permiten bucles que dependan de la escena
- Recursión limitada por profundidad constante

### 8.3 COMPATIBILIDAD TENSORIAL
- **Geometría**: Solo opera sobre índices 0-3 [x,y,z,w]
- **Color**: Solo opera sobre índices 4-7 [r,g,b,a]
- **Separación estricta**: Nunca mezclar ambos dominios

### 8.4 OPERADORES DISPONIBLES
```
// Funciones matemáticas básicas
sin(), cos(), tan(), asin(), acos(), atan()
pow(), sqrt(), exp(), log(), abs()

// Operadores de comparación
max(), min(), clamp(), step(), smoothstep()

// Álgebra lineal (si el evaluador las expone)
dot(), length(), normalize(), cross()

// Variables de entorno
x, y, z, w, pi, e

// Variables de contexto óptico
nx, ny, nz (normal)
lx, ly, lz (luz)
vx, vy, vz (cámara/vista)
```

## 9. PRÓXIMOS PASOS

1. **Identificar ecuaciones canónicas** para cada subcampo prioritario
2. **Reducir a forma cerrada** evaluable con operadores disponibles
3. **Traducir a álgebra hermética** con `def` y `let`
4. **Implementar librerías std_math_*.hm** siguiendo plantilla
5. **Validar con escenas de prueba** que demuestren funcionalidad

---

**NOTA:** Esta jerarquía representa los fundamentos matemáticos que deben implementarse como librerías `.hm` puras, sin modificar el código C++ existente. Cada función debe derivarse de principios matemáticos canónicos y reducirse a expresiones evaluables con los operadores disponibles en el lenguaje.