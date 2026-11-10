#pragma once
#include <cstdint>
#include <string>

namespace herm {

float evalExpression(const std::string& expr, float w);
float evalExpression(const std::string& expr, float x, float y, float z, float w,
                     float nx = 0.0f, float ny = 0.0f, float nz = 0.0f,
                     float vx = 0.0f, float vy = 0.0f, float vz = 0.0f,
                     float lx = 0.0f, float ly = 0.0f, float lz = 0.0f,
                     int depth = 0);

// 2.0c.3b (DL-36): compilador de expresiones a bytecode ONT postfijo.
// Replica la gramatica de ExprParser; no anade sintaxis.
class OntExprSink {
public:
    virtual ~OntExprSink() = default;
    virtual void emitOp(uint8_t op) = 0;
    virtual void emitConst(float v) = 0;
};

// Compila `expr` y emite instrucciones [op][f32] de 5 bytes al sink.
// Lanza std::runtime_error con la expresion y la posicion ante cualquier
// construccion no representable (% , tanh, sample, variables de sombreado,
// funcion/variable desconocida) o gramatica invalida.
void compileOntExpression(const std::string& expr, OntExprSink& sink);

} // namespace herm
