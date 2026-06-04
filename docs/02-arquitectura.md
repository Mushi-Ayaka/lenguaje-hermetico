# Arquitectura del Sistema

## 1. Pipeline principal

```
.herm → [herm] → .rih → [operador_tb] → .tb → [render.exe] → .png
                  ↘ --render → .png (directo, vía stb_image_write)
```

## 2. Pipeline alterno (proyecto aparte)

```
.herm → [herm] → .rih → [driver_mlir en Lienzo y renderizado/] → .gte → .png
```

El pipeline MLIR vive en `Lienzo y renderizado/` — proyecto independiente que comparte el RIH como contrato.

## 3. Componentes

### 3.1 `herm` — Compilador unificado

| Fase | Entrada | Salida | Descripción |
|------|---------|--------|-------------|
| **Parse** | `.herm` | AST | Lexer + parser |
| **Resolver** | AST | Grafo resuelto | SDFs, herencia, `let`, `def`/`instance` |
| **Emit** | Grafo | `.rih` JSON | RIH (Representación Intermedia Hermética) |
| **Render** | Grafo | `.png` | Pipeline directo |

**Flags:**

| Flag | Descripción |
|------|-------------|
| `-o <archivo>` | Archivo de salida |
| `--frames N-M` | Rango de fotogramas (eje W) |
| `--render` | Pipeline completo hasta .png |
| `--bin` | Salida .rib (formato binario, **sin lector** — no usar) |

### 3.2 `operador_tb` — Backend standalone

Lee RIH JSON, renderiza por ray marching, empaqueta en .tb.

### 3.3 `render.exe` — Visualizador de .tb

Herramienta existente del pipeline original.

## 4. RIH (Representación Intermedia Hermética)

Contrato entre compilador y backends. Formato JSON con:

- `version`, `scene` (cámara, ejes, fondo)
- `materials[]`: tabla de materiales definida por su Plano Tensorial 1x8 (`tensor[8]`) interactivo
- `defs[]`: definiciones reutilizables
- `nodes[]`: sdf, group, instance
- `lights[]`: luces direccionales y puntuales

## 5. Dependencias

- Visual Studio 2022 Build Tools
- C++17 estándar
- `stb_image_write.h` (en `Lienzo y renderizado/3rdparty/`)
- `render.exe` (.tb → .png)
