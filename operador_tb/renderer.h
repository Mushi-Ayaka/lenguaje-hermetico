#pragma once
#include "../contrato/rih.h"
#include "tb_writer.h"
#include <string>
#include <vector>

// Render RIH scene to pixel buffer
std::vector<Rgba> renderRih(const herm::Rih& rih, int width, int height);
