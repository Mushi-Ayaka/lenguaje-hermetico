#pragma once
#include "../contrato/rih.h"
#include <string>
#include <vector>

namespace herm {

struct Rgba {
    uint8_t r, g, b, a;
};

struct HitInfo {
    bool hit;
    float dist;
    Vec3 point;
    Vec3 normal;
    u32 material_id;
};

// Ray march the RIH scene at the given resolution, return pixel buffer
std::vector<Rgba> renderRih(const Rih& rih, int width, int height);

// Write pixel buffer as GTE file (8x8 blocks, compatible with gte.exe)
bool writeGte(const Rih& rih, const std::vector<Rgba>& pixels,
              int width, int height, const std::string& path);

// Write pixel buffer as PNG file
bool writePng(const std::vector<Rgba>& pixels,
              int width, int height, const std::string& path);

} // namespace herm
