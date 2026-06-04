#pragma once
#include <string>
#include <vector>
#include <cstdint>

struct Rgba { uint8_t r, g, b, a; };

bool writeTb(const std::vector<Rgba>& pixels, int width, int height,
             const std::string& path);
