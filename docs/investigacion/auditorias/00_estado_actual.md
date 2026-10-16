# AUDITORÍA INICIAL - ESTADO DEL PROYECTO LENGUAJE HERMÉTICO
**Fecha:** 5 de junio de 2026

## 1. ANÁLISIS DE IMPLEMENTACIÓN ACTUAL

### 1.1 Pipeline Funcional (COMPLETO)
```
.herm → herm.exe → .rih (JSON) → operador_tb → .tb → render.exe → .png
```

### 1.2 Componentes Implementados en C++ (HARDCODEADOS)
1. **SDFs Primitivas**: sphere, box, cylinder, torus, cone, plane, capsule, rounded_box, octahedron
2. **Operadores SDF**: union, subtract, intersect, smooth_*, repeat, twist, elongate, displace
3. **Sistema de Luces**: directional + point con shadow
4. **Materiales PBR**: base_color, roughness, metallic, emission, ior, transmission, opacity
5. **Evaluador de Expresiones**: x, y, z, w, t, pi, e + funciones matemáticas

### 1.3 Librería Existente (VIOLA PRINCIPIO HERMÉTICO)
- `materiales.hm`: ~40 materiales definidos EMPÍRICAMENTE
- **PROBLEMA**: Define colores y propiedades sin derivarlas de leyes físicas/matemáticas
- **EJEMPLO**: `material "oro" { base_color: [1.0, 0.84, 0.0]; roughness: 0.1; metallic: 1.0; }`
- **FALTA**: NO se deriva de leyes ópticas (Fresnel, dispersión, espectro atómico)

### 1.4 Lo que NO está implementado según protocolo:
1. **Ninguna librería std_*.hm** (0/22 librerías del protocolo)
2. **Árbol Ontológico**: No existe jerarquía científica
3. **Derivación Física-Matemática**: Todo está hardcodeado
4. **Compatibilidad Tensorial Estricta**: No se respeta separación [x,y,z,w] vs [r,g,b,a]

## 2. PROBLEMA FUNDAMENTAL IDENTIFICADO

### 2.1 Violación del Principio Hermético
- **Principio**: Todo debe derivarse de leyes fundamentales matemáticas/físicas
- **Realidad Actual**: Materiales definidos por valores RGB arbitrarios
- **Ejemplo Crítico**: `oro` debería derivar su color de:
  1. Ecuaciones de Fresnel para metales
  2. Espectro de reflectividad del oro real
  3. Ecuaciones de dispersión electrónica
  4. Índice de refracción complejo del oro

### 2.2 Deuda Técnica Documentada
Según `docs/contrato-ontologico.md`:
- **FALTA**: Que las SDFs sean expresiones evaluables en vez de switch en C++
- **FALTA**: Que las primitivas vivan en `.hm` como `def`s
- **FALTA**: `x, y, z` ligados al punto del rayo (hoy solo `w`)
- **FALTA**: `hash()` determinista para formas naturales

## 3. JERARQUÍA REQUERIDA SEGÚN PROTOCOLO

### Capa 1: Base Matemática (TRONCO)
```
1.1 Lógica Matemática
1.2 Álgebra Abstracta y Geometría Diferencial (Deformación de [x,y,z,w])
1.3 Cálculo Avanzado y Ecuaciones Diferenciales
1.4 Teoría de la Probabilidad y Estadística
1.5 Teoría de la Información y Sistemas Dinámicos
```

### Capa 2: Física (RAÍCES)
```
2.1 Física Cuántica y Mecánica de Matrices
2.2 Teoría Cuántica de Campos
2.3 Electrodinámica Cuántica
2.4 Cromodinámica Cuántica
2.5 Relatividad General y Especial
2.6 Mecánica Estadística
2.7 Termodinámica de Sistemas Fuera del Equilibrio
2.8 Física del Estado Sólido y Materia Condensada
```

### Capa 3: Física-Química (PUENTE ATÓMICO)
```
3.1 Física Atómica
3.2 Física Molecular
3.3 Química Cuántica
3.4 Fisicoquímica
3.5 Espectroscopia Molecular ← CRÍTICO PARA COLOR
3.6 Mecánica Estadística Molecular
3.7 Cinética Química y Termodinámica Química
3.8 Electroquímica
```

### Capa 4: Química (ÓPTICA/MATERIAL)
```
4.1 Química Inorgánica
4.2 Química Orgánica
4.3 Química de Coordinación
4.4 Química Supramolecular
4.5 Química de Polímeros
4.6 Cristalografía
4.7 Química Teórica y Computacional
```

### Capa 5: Química-Biología (PUENTE VIDA)
```
5.1 Bioquímica
5.2 Bioquímica Estructural
5.3 Biología Molecular
5.4 Biología Química
5.5 Biofísica Molecular
5.6 Enzimología
5.7 Genómica Química y Metabolómica
```

### Capa 6: Biología (PATRONES/COMPLEJIDAD)
```
6.1 Biología Celular y Citología
6.2 Genética y Epigenética
6.3 Biología del Desarrollo y Morfogénesis
6.4 Fisiología (Sistémica y Celular)
6.5 Neurobiología y Neurociencia Computacional
6.6 Biología Evolutiva
6.7 Ecología de Poblaciones y Comunidades
6.8 Biología de Sistemas
```

### Capa 7: Macro-Sistémica (MUNDO)
```
7.1 Geología y Geomorfología
7.2 Meteorología y Climatología
7.3 Oceanografía y Dinámica de Fluidos planetaria
7.4 Ecología de Paisajes y Biomas
```

### Capa 8: Cosmológico (UNIVERSO)
```
8.1 Mecánica Celeste y Órbitas
8.2 Astrofísica Estelar
8.3 Cosmología Física y Nebulosas
```

## 4. PRIORIDADES DE DESARROLLO

### PRIORIDAD 1: Base Matemática (Capa 1)
- **Por qué**: Sin matemáticas, no hay física
- **Qué incluye**: Álgebra vectorial, cálculo, probabilidad
- **Dependencias**: Ninguna (capa base)

### PRIORIDAD 2: Física Fundamental (Capa 2)
- **Por qué**: Leyes del universo físico
- **Qué incluye**: Relatividad, mecánica cuántica, termodinámica
- **Dependencias**: Capa 1 (matemáticas)

### PRIORIDAD 3: Óptica/Color (Capas 3-4)
- **Por qué**: Para reemplazar `materiales.hm` con derivación científica
- **Qué incluye**: Espectroscopia, química óptica, cristalografía
- **Dependencias**: Capas 1-2

## 5. PRÓXIMOS PASOS

1. **Crear jerarquía detallada** de subcampos por capa
2. **Identificar ecuaciones canónicas** para cada subcampo
3. **Reducir a forma cerrada** evaluable en tiempo real
4. **Traducir a álgebra hermética** (solo operadores permitidos)
5. **Implementar std_*.hm** siguiendo plantilla canónica
6. **Validar con escenas de prueba** `.herm`

## 6. RESTRICCIONES CLAVE

1. **PROHIBIDO estado mutable**
2. **PROHIBIDO modificar código C++**
3. **HERENCIA OBLIGATORIA** de capas inferiores
4. **COMPATIBILIDAD TENSORIAL ESTRICTA**:
   - Geometría: solo modifica `[x, y, z, w]`
   - Color: solo modifica `[r, g, b, a]`
   - NUNCA mezclar ambos

---

**CONCLUSIÓN:** El proyecto tiene un pipeline funcional pero carece completamente de la jerarquía científica requerida. La librería existente `materiales.hm` es anticientífica y debe ser reemplazada por derivaciones físicas/matemáticas.