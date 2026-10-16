# INVESTIGACIÓN CIENTÍFICA - ÁLGEBRA VECTORIAL 3D/4D
**Para:** std_math_algebra.hm (Capa 1.2)
**Fecha:** 5 de junio de 2026
**Autor:** Kiro - Agente IA

## 1. ECUACIONES CANÓNICAS (Fase 1 del Protocolo)

### 1.1 PRODUCTO PUNTO (DOT PRODUCT) - ℝⁿ
**Ecuación canónica:** Para vectores **a** = (a₁, a₂, ..., aₙ) y **b** = (b₁, b₂, ..., bₙ)

```
a · b = Σᵢ₌₁ⁿ aᵢbᵢ = a₁b₁ + a₂b₂ + ... + aₙbₙ
```

**Propiedades fundamentales:**
1. Conmutativa: **a · b** = **b · a**
2. Distributiva: **a · (b + c)** = **a · b** + **a · c**
3. Bilineal: (α**a**) · **b** = α(**a · b**) = **a ·** (α**b**)

**Interpretación geométrica:**
- **a · b** = ‖a‖‖b‖ cos θ (donde θ es el ángulo entre los vectores)
- Si **a · b** = 0 → vectores ortogonales (θ = 90°)

### 1.2 NORMA (MAGNITUD) - ℝⁿ
**Ecuación canónica:** Para vector **v** = (v₁, v₂, ..., vₙ)

```
‖v‖ = √(v · v) = √(Σᵢ₌₁ⁿ vᵢ²) = √(v₁² + v₂² + ... + vₙ²)
```

**Propiedades:**
1. ‖v‖ ≥ 0 (no negativa)
2. ‖v‖ = 0 ↔ **v** = **0**
3. ‖αv‖ = |α|‖v‖ (homogeneidad absoluta)
4. ‖v + w‖ ≤ ‖v‖ + ‖w‖ (desigualdad triangular)

### 1.3 NORMALIZACIÓN - ℝⁿ
**Ecuación canónica:** Para vector **v** ≠ **0**

```
v̂ = v / ‖v‖ = (v₁/‖v‖, v₂/‖v‖, ..., vₙ/‖v‖)
```

**Propiedad:** ‖v̂‖ = 1 (vector unitario)

### 1.4 PRODUCTO CRUZ (CROSS PRODUCT) - ℝ³ SOLO
**Ecuación canónica:** Para vectores **a** = (a₁, a₂, a₃) y **b** = (b₁, b₂, b₃)

```
a × b = (a₂b₃ - a₃b₂, a₃b₁ - a₁b₃, a₁b₂ - a₂b₁)
```

**Forma de determinante (mnemotécnica):**
```
        | i  j  k  |
a × b = | a₁ a₂ a₃ | = i(a₂b₃ - a₃b₂) - j(a₁b₃ - a₃b₁) + k(a₁b₂ - a₂b₁)
        | b₁ b₂ b₃ |
```

**Propiedades fundamentales:**
1. Anticonmutativa: **a × b** = -(**b × a**)
2. Distributiva: **a × (b + c)** = **a × b** + **a × c**
3. Bilineal: (α**a**) × **b** = α(**a × b**) = **a ×** (α**b**)
4. Ortogonalidad: **a · (a × b)** = **b · (a × b)** = 0
5. Magnitud: ‖a × b‖ = ‖a‖‖b‖ sin θ (área del paralelogramo)

### 1.5 PRODUCTO CRUZ GENERALIZADO - ℝ⁷ (Opcional)
**Nota:** El producto cruz solo está definido en ℝ³ y ℝ⁷. Para ℝ⁴, necesitamos usar el **producto exterior (wedge product)**.

### 1.6 PRODUCTO EXTERIOR (WEDGE PRODUCT) - ℝⁿ (Generalización)
**Ecuación canónica:** Para vectores **a**, **b** en ℝⁿ

```
a ∧ b = a ⊗ b - b ⊗ a  (producto tensorial antisimetrizado)
```

En componentes (para ℝ⁴):
```
(a ∧ b)ᵢⱼ = aᵢbⱼ - aⱼbᵢ  (tensor antisimétrico de rango 2)
```

**Propiedad:** **a ∧ b** = -(**b ∧ a**) (antisimétrico)

## 2. REDUCCIÓN A FORMA CERRADA (Fase 2 del Protocolio)

### 2.1 PRODUCTO PUNTO 3D (Forma cerrada)
```
dot3(a, b) = a.x*b.x + a.y*b.y + a.z*b.z
```

**Implementación directa:** Suma de productos componente a componente.

### 2.2 PRODUCTO PUNTO 4D (Forma cerrada)
```
dot4(a, b) = a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w
```

### 2.3 NORMA 3D (Forma cerrada)
```
length3(v) = sqrt(v.x*v.x + v.y*v.y + v.z*v.z)
```

**Optimización:** Para comparaciones (sin raíz cuadrada):
```
length_squared3(v) = v.x*v.x + v.y*v.y + v.z*v.z
```

### 2.4 NORMA 4D (Forma cerrada)
```
length4(v) = sqrt(v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w)
```

### 2.5 NORMALIZACIÓN 3D (Forma cerrada)
```
normalize3(v) = v / sqrt(v.x*v.x + v.y*v.y + v.z*v.z)
```

**Estabilidad numérica:** Evitar división por cero:
```
if (length_squared < ε²) return zero_vector
else return v / sqrt(length_squared)
```

### 2.6 PRODUCTO CRUZ 3D (Forma cerrada)
```
cross3(a, b) = (a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x)
```

### 2.7 ÁNGULO ENTRE VECTORES (Forma cerrada usando producto punto)
```
angle_between(a, b) = acos(dot3(normalize3(a), normalize3(b)))
```

**Límite numérico:** `clamp(dot, -1, 1)` antes de `acos()`

### 2.8 PROYECCIÓN DE VECTOR (Forma cerrada)
```
project(a, b) = (dot3(a, b) / dot3(b, b)) * b
```

### 2.9 COMPONENTE REJECT (ORTONORMAL) (Forma cerrada)
```
reject(a, b) = a - project(a, b)
```

## 3. TRADUCCIÓN AL ÁLGEBRA HERMÉTICA (Fase 3 del Protocolo)

### 3.1 OPERADORES DISPONIBLES EN LENGUAJE HERMÉTICO
```
sin(), cos(), tan(), asin(), acos(), atan()
pow(), sqrt(), exp(), log(), abs()
max(), min(), clamp(), step(), smoothstep()
// Álgebra lineal (SI el evaluador las expone):
dot(), length(), normalize(), cross()
// Variables: x, y, z, w, pi, e
// Contexto: nx, ny, nz, lx, ly, lz, vx, vy, vz
```

### 3.2 SUPOSICIÓN CRÍTICA
**PROBLEMA:** La documentación dice "Álgebra lineal: `dot, length, normalize, cross` *(si el evaluador las expone)*"

**ANÁLISIS:** Dado que las SDFs necesitan calcular normales (gradiente), es probable que `dot()`, `length()`, etc. estén expuestas. Pero debemos verificar.

**PLAN B:** Si NO están expuestas, debemos implementarlas usando operadores básicos.

### 3.3 IMPLEMENTACIÓN USANDO OPERADORES BÁSICOS (si es necesario)

#### Producto Punto 3D (sin `dot()`):
```
def "dot3"(ax, ay, az, bx, by, bz) {
    return ax*bx + ay*by + az*bz;
}
```

#### Longitud 3D (sin `length()`):
```
def "length3"(x, y, z) {
    return sqrt(x*x + y*y + z*z);
}
```

#### Normalización 3D (sin `normalize()`):
```
def "normalize3"(x, y, z) {
    let len = sqrt(x*x + y*y + z*z);
    // Evitar división por cero
    return if(len > 0.0001) then [x/len, y/len, z/len] else [0, 0, 0];
}
```

#### Producto Cruz 3D (sin `cross()`):
```
def "cross3"(ax, ay, az, bx, by, bz) {
    let cx = ay*bz - az*by;
    let cy = az*bx - ax*bz;
    let cz = ax*by - ay*bx;
    return [cx, cy, cz];
}
```

## 4. ERRORES DE APROXIMACIÓN Y ESTABILIDAD NUMÉRICA

### 4.1 ERRORES COMUNES
1. **División por cero** en normalización
2. **Overflow/underflow** en cálculo de normas
3. **Pérdida de precisión** en `acos(dot)` cerca de ±1
4. **Inestabilidad** en productos cruz con vectores casi paralelos

### 4.2 SOLUCIONES
1. **Umbral ε**: `if (length_squared < 1e-10) return zero`
2. **Clamping**: `clamp(dot, -1+ε, 1-ε)` antes de `acos()`
3. **Forma estable**: Usar `atan2(sinθ, cosθ)` para ángulos
4. **Doble precisión**: Si disponible en evaluador

## 5. APLICACIONES EN LENGUAJE HERMÉTICO

### 5.1 CÁLCULO DE NORMALES DE SUPERFICIE
Para SDF Φ(x,y,z), la normal es:
```
n = normalize(∇Φ) ≈ normalize([Φ(x+ε,y,z)-Φ(x-ε,y,z), 
                                Φ(x,y+ε,z)-Φ(x,y-ε,z),
                                Φ(x,y,z+ε)-Φ(x,y,z-ε)] / (2ε))
```

### 5.2 ILUMINACIÓN (BLINN-PHONG)
```
// Vector de vista (normalizado)
view_dir = normalize(camera_pos - surface_point)

// Vector de luz (normalizado)
light_dir = normalize(light_pos - surface_point)

// Medio vector (Blinn-Phong)
half_vector = normalize(view_dir + light_dir)

// Cálculos de intensidad
diffuse = max(0, dot(normal, light_dir))
specular = pow(max(0, dot(normal, half_vector)), shininess)
```

### 5.3 REFLEXIONES Y REFRACCIONES
```
// Reflexión
reflected = incident - 2 * dot(incident, normal) * normal

// Refracción (ley de Snell)
// Necesita cálculos con producto punto y sqrt
```

## 6. DEPENDENCIAS Y COMPATIBILIDAD

### 6.1 DEPENDENCIAS
- **Ninguna** (capa base matemática)
- Operadores matemáticos básicos: `+`, `-`, `*`, `/`, `sqrt()`

### 6.2 COMPATIBILIDAD TENSORIAL
- **Geometría**: Solo opera sobre `[x, y, z, w]` (índices 0-3)
- **NO opera sobre color**: `[r, g, b, a]` (índices 4-7)
- **Separación estricta** mantenida

## 7. FUENTES CIENTÍFICAS

1. **Álgebra Lineal**: Strang, G. "Linear Algebra and Its Applications"
2. **Cálculo Vectorial**: Marsden & Tromba "Vector Calculus"
3. **Geometría Computacional**: Schneider & Eberly "Geometric Tools for Computer Graphics"
4. **Formularios**: "CRC Standard Mathematical Tables and Formulae"

## 8. PRÓXIMOS PASOS

1. **Verificar operadores disponibles** en evaluador hermético
2. **Implementar funciones básicas** según disponibilidad
3. **Crear librería std_math_algebra.hm**
4. **Probar con escena de validación**
5. **Documentar métricas de rendimiento**

---

**NOTA:** Esta investigación cumple con la Fase 1 (Investigación Científica) del Protocolo. Las ecuaciones canónicas están identificadas y listas para reducción a forma cerrada evaluable en tiempo real.