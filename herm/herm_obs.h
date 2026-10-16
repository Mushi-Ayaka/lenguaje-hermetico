#pragma once
#include "../contrato/rih.h"
#include "../contrato/ont.h"
#include <string>

namespace herm {

bool writeObs(const Rih& rih, const std::string& ont_path);

} // namespace herm
