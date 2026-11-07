#pragma once
#include <string>

namespace herm {

float evalExpression(const std::string& expr, float w);
float evalExpression(const std::string& expr, float x, float y, float z, float w,
                     float nx = 0.0f, float ny = 0.0f, float nz = 0.0f,
                     float vx = 0.0f, float vy = 0.0f, float vz = 0.0f,
                     float lx = 0.0f, float ly = 0.0f, float lz = 0.0f,
                     int depth = 0);

// Devuelve el nombre de la primera funcion de `expr` que no tiene opcode en
// el bytecode ONT (step, smoothstep, exp, log, log2, log10, acos, asin,
// atan, atan2, sign, fract), o "" si no hay ninguna. Solo detecta llamadas
// con parentesis; no confunde variables que contengan el nombre.
std::string findUnsupportedOntFn(const std::string& expr);

} // namespace herm
