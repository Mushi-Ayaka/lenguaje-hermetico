#pragma once
#include "rih.h"
#include <string>
#include <vector>

// ============================================================================
// CONTRATO DE LA HERRAMIENTA herm
// Compilador + Resolvedor del Lenguaje Hermético.
// Lee .herm y emite RIH (JSON o binario).
// ============================================================================

namespace herm {

// ---------------------------------------------------------------------------
// Configuración de compilación
// ---------------------------------------------------------------------------

struct HermConfig {
    std::string input_path;        // .herm
    std::string output_path;       // .rih, .rib, .tb, o .gte
    bool        binary_output  = false; // .rib en lugar de .rih
    bool        emit_tb        = false; // RIH → .tb directo
    bool        emit_gte       = false; // RIH → .gte
    bool        render         = false; // todo hasta .png
    u32         frame_start    = 0;     // para animaciones
    u32         frame_end      = 0;     // 0 = usar W de la escena
    f32         t_scale        = 1.0f;  // escala temporal
};

// ---------------------------------------------------------------------------
// Error handling
// ---------------------------------------------------------------------------

struct HermError {
    u32         line;
    u32         column;
    std::string message;
    std::string context; // línea de código fuente
};

struct HermResult {
    bool                  success;
    std::vector<HermError> errors;
    std::string           output_path; // ruta del archivo generado
    u32                   frames_rendered = 0;
    f64                   elapsed_ms = 0.0;
};

// ---------------------------------------------------------------------------
// API principal
// ---------------------------------------------------------------------------

// Compila un archivo .herm y genera la salida según config
HermResult compile(const HermConfig& config);

// Compila desde string (para integración)
HermResult compileFromString(const std::string& source, const HermConfig& config);

// Lee RIH desde archivo
Rih loadRih(const std::string& path);

// Escribe RIH en formato JSON
bool writeRihJson(const Rih& rih, const std::string& path);

// Escribe RIH en formato binario .rib
bool writeRihBinary(const Rih& rih, const std::string& path);

// ---------------------------------------------------------------------------
// Flags de línea de comandos (documentación)
// ---------------------------------------------------------------------------

// herm entrada.herm -o salida.rih       # RIH en JSON
// herm entrada.herm -o salida.rib --bin # RIH en binario
// herm entrada.herm -o salida.tb  --tb  # RIH → TB directo
// herm entrada.herm -o salida.gte       # RIH → GTE
// herm entrada.herm --render            # Todo hasta PNG
// herm entrada.herm --frames 0-59       # Rango de fotogramas

} // namespace herm
