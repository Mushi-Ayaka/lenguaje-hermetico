#include "tb_writer.h"
#include <cstdio>
#include <cstring>
#include <cmath>
#include <algorithm>

#pragma pack(push, 1)
struct BlockHeader {
    uint16_t px, py;
    uint8_t  size;
    uint8_t  type;
    uint16_t data_len;
    uint8_t  hint;
};
#pragma pack(pop)

enum BlockType : uint8_t {
    BLOCK_RAW        = 0x00,
    BLOCK_P0         = 0x01,
    BLOCK_END        = 0xFF
};

bool writeTb(const std::vector<Rgba>& pixels, int width, int height,
             const std::string& path) {
    FILE* f = fopen(path.c_str(), "wb");
    if (!f) return false;

    int block_size = 8;
    int blocks_x = (width + block_size - 1) / block_size;
    int blocks_y = (height + block_size - 1) / block_size;

    for (int by = 0; by < blocks_y; by++) {
        for (int bx = 0; bx < blocks_x; bx++) {
            int x0 = bx * block_size;
            int y0 = by * block_size;
            int x1 = std::min(x0 + block_size, width);
            int y1 = std::min(y0 + block_size, height);
            int bw = x1 - x0;
            int bh = y1 - y0;

            // Check if all pixels are the same color
            bool uniform = true;
            Rgba first = pixels[y0 * width + x0];
            for (int py = y0; py < y1 && uniform; py++) {
                for (int px = x0; px < x1; px++) {
                    auto& p = pixels[py * width + px];
                    if (p.r != first.r || p.g != first.g ||
                        p.b != first.b || p.a != first.a) {
                        uniform = false;
                        break;
                    }
                }
            }

            if (uniform) {
                // BLOCK_P0
                BlockHeader hdr;
                hdr.px = (uint16_t)x0;
                hdr.py = (uint16_t)y0;
                hdr.size = (uint8_t)block_size;
                hdr.type = BLOCK_P0;
                hdr.data_len = 4;
                hdr.hint = 0;
                fwrite(&hdr, sizeof(hdr), 1, f);
                uint8_t payload[4] = {first.r, first.g, first.b, first.a};
                fwrite(payload, 1, 4, f);
            } else {
                // BLOCK_RAW
                int bytes = bw * bh * 4;
                BlockHeader hdr;
                hdr.px = (uint16_t)x0;
                hdr.py = (uint16_t)y0;
                hdr.size = (uint8_t)block_size;
                hdr.type = BLOCK_RAW;
                hdr.data_len = (uint16_t)bytes;
                hdr.hint = 0;
                fwrite(&hdr, sizeof(hdr), 1, f);
                for (int py = y0; py < y1; py++) {
                    for (int px = x0; px < x1; px++) {
                        auto& p = pixels[py * width + px];
                        uint8_t rgba[4] = {p.r, p.g, p.b, p.a};
                        fwrite(rgba, 1, 4, f);
                    }
                }
            }
        }
    }

    // BLOCK_END
    BlockHeader end_hdr = {};
    end_hdr.type = BLOCK_END;
    fwrite(&end_hdr, sizeof(end_hdr), 1, f);

    fclose(f);
    return true;
}
