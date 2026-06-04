#include "rih_reader.h"
#include "renderer.h"
#include "tb_writer.h"
#include <cstdio>
#include <string>
#include <chrono>

int main(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "Uso: operador_tb entrada.rih salida.tb\n");
        return 1;
    }

    std::string in_path = argv[1];
    std::string out_path = argv[2];

    auto t0 = std::chrono::high_resolution_clock::now();

    // Load RIH
    herm::Rih rih = herm::loadRih(in_path);
    if (rih.materials.empty() && rih.nodes.empty()) {
        fprintf(stderr, "[operador_tb] error: no se pudo cargar %s\n", in_path.c_str());
        return 1;
    }

    int w = (int)rih.scene.width;
    int h = (int)rih.scene.height;
    if (w == 0 || h == 0) { w = 800; h = 600; }

    auto t1 = std::chrono::high_resolution_clock::now();

    // Render
    auto pixels = renderRih(rih, w, h);

    auto t2 = std::chrono::high_resolution_clock::now();

    // Write TB
    if (!writeTb(pixels, w, h, out_path)) {
        fprintf(stderr, "[operador_tb] error: no se pudo escribir %s\n", out_path.c_str());
        return 1;
    }

    auto t3 = std::chrono::high_resolution_clock::now();

    auto load_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    auto render_ms = std::chrono::duration<double, std::milli>(t2 - t1).count();
    auto write_ms = std::chrono::duration<double, std::milli>(t3 - t2).count();

    fprintf(stderr, "[operador_tb] OK: %s -> %s (%dx%d, %.0fms load + %.0fms render + %.0fms write)\n",
            in_path.c_str(), out_path.c_str(), w, h, load_ms, render_ms, write_ms);
    return 0;
}
