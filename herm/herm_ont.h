#pragma once
#include "../contrato/rih.h"
#include "../contrato/ont.h"
#include <string>

namespace herm {

// Write .ont binary from resolved RIH
// Precomputes BVH, d_min, L, converts SDF trees to single bytecode expressions
bool writeOnt(const Rih& rih, const std::string& path);

// 2.0c.1 — comprueba que todos los parametros SDF se puedan representar en
// el bytecode ONT. Lanza std::runtime_error con primitiva, parametro y linea.
// Debe llamarse sobre el Rih resuelto, ANTES de evaluateFrame() (que pliega
// las expresiones con x=y=z=0).
void validateOntCompatibility(const Rih& rih);

} // namespace herm
