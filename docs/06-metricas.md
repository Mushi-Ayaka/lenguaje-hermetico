# Métricas y Benchmarks

## 1. Tiempos de Compilación

Escenas de prueba medidas en PC de referencia (CPU, build `/O2`):

| Escena | Nodos | Fotogramas | Parse | Eval+Emit | Total |
|--------|-------|------------|-------|-----------|-------|
| bodegon | 11 | 1 | 0ms | 0ms | 0ms |
| torre | 30 | 1 | 1ms | 3ms | 4ms |
| instancias | 18 | 1 | 1ms | 2ms | 3ms |
| animada | 14 | 30 | 0ms | 73ms | 73ms |
| teclado | 40 | 33 | 0ms | 84ms | 84ms |

## 2. Tiempos de Render

Pipeline: `.herm → herm --render → .png`. Resolución 800×600.

### 2.1 Escenas simples (build `/O2`)

| Escena | Nodos | Resolución | Tiempo |
|--------|-------|------------|--------|
| bodegon | 11 | 800×600 | 2.5s |
| animada (frame 0) | 14 | 800×600 | 0.94s |
| volumen (niebla + esfera) | 2 | 400×300 | 0.125s |

### 2.2 Escena compleja (teclado, 40 nodos)

| Resolución | Tiempo/frame | 33 frames |
|------------|-------------|-----------|
| 800×600 | ~27-31s | ~15-17 min |
| 1200×800 | ~50s | ~28 min |

## 3. Pipeline operador_tb

| Escena | Resolución | Tiempo |
|--------|-----------|--------|
| bodegon (RIH → .tb) | 800×600 | 2.7s |
| animada_0000 | 800×600 | 2.2s |

## 4. Comparativa: Debug vs Optimizado

Compilación con `/O2` vs sin optimizar:

| Escena | Debug | /O2 | Factor |
|--------|-------|-----|--------|
| bodegon (herm --render) | 58s | 2.5s | ~23× |
| bodegon (operador_tb) | 39s | 2.7s | ~14× |

La mejora es drástica debido al uso intensivo de templates (unordered_map, lambdas, funciones matemáticas) que el optimizador inlinea y vectoriza.

## 5. Memoria

| Componente | Uso típico | Pico |
|------------|-----------|------|
| herm (parse + eval) | ~5-15 MB | ~30 MB (teclado 33 frames) |
| operador_tb (render 800×600) | ~10-20 MB | ~50 MB (buffer RGBA + grafo) |
| RIH JSON (teclado 33 frames) | ~200 KB | — |
| RIH binario (.rib) estimado | ~50 KB | — |

## 6. Calidad de Render

### 6.1 Sphere Tracing

| Parámetro | Valor | Efecto |
|-----------|-------|--------|
| MAX_STEPS | 128 | Precisión vs velocidad |
| EPS (hit) | 1e-4 | Tolerancia de superficie |
| MAX_DIST | 100.0 | Distancia máxima del rayo |

### 6.2 Sombra

| Parámetro | Valor |
|-----------|-------|
| SHADOW_BIAS | 0.01 |
| SHADOW_STEPS | 32 |
| SHADOW_MAX_DIST | 5.0 |
| Factor oscuridad | 0.3 |

### 6.3 Iluminación

| Componente | Factor |
|-----------|--------|
| Ambiente | 0.1 |
| Difusa (Lambert) | 0.6 |
| Especular (Blinn-Phong) | 0.3 |
| Brillo (shininess) | 32 |

### 6.4 Penalización de Evaluación Tensorial (v0.14)

Al unificar las propiedades aisladas de color en el Plano Tensorial 1x8, los componentes ópticos (`r`, `g`, `b`, `a`) se obtienen como expresiones de índice `[4..7]` por cada colisión de rayo de luz en `shadeWithLights()`. Este lookup dinámico incrementa marginalmente los milisegundos totales del pase de sombreado (~1% a 2% de overhead en escenas densas), pero está más que justificado por el desbloqueo semántico y arquitectónico.

## 7. Eficiencia del Formato .tb

Blanco teórico para 800×600 (480,000 píxeles):

| Métrica | Valor |
|---------|-------|
| Bloques 8×8 | 7,500 |
| P0 (color sólido) | ~1,900 (escena bodegon) |
| PAL_RGBA | ~1,500 |
| RAW | ~100 |
| SPAN groups | ~95 |
| Tamaño archivo | ~20-50 KB típico |
