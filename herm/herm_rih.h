#pragma once
#include "../contrato/rih.h"
#include <string>

namespace herm {

bool writeRihJson(const Rih& rih, const std::string& path);
bool writeRihBinary(const Rih& rih, const std::string& path);

std::string rihToJsonString(const Rih& rih);

// Evalúa todas las expresiones con t en un RIH, produce un RIH sin expresiones.
// Con fold_sdf_params=false las transformaciones se pliegan pero los
// parametros SDF conservan su expresion (2.0c.3b, DL-36: los compila writeOnt).
Rih evaluateFrame(const Rih& rih, float t, bool fold_sdf_params = true);

// Genera nombre de archivo con padding de ceros: "output_0003.rih"
std::string framePath(const std::string& prefix, int frame, bool binary);

} // namespace herm
