#pragma once
#include "../contrato/rih.h"
#include "../contrato/ont.h"
#include <string>

namespace herm {

// Write .ont binary from resolved RIH
// Precomputes BVH, d_min, L, converts SDF trees to single bytecode expressions
bool writeOnt(const Rih& rih, const std::string& path);

} // namespace herm
