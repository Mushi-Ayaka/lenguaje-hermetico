#include "herm_lexer.h"
#include "herm_parser.h"
#include "herm_resolver.h"
#include "herm_rih.h"
#include "herm_render.h"
#include "herm_ont.h"
#include "herm_obs.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <cstdint>
#include <filesystem>
namespace fs = std::filesystem;

static void printUsage() {
    std::cerr << "Uso: herm <archivo.herm> [opciones]\n";
    std::cerr << "\nOpciones:\n";
    std::cerr << "  -o <archivo>    Archivo de salida (prefijo con --frames)\n";
    std::cerr << "  --bin           Salida binaria (.rib)\n";
    std::cerr << "  --tb            Emitir .tb directo\n";
    std::cerr << "  --gte           Emitir .gte\n";
    std::cerr << "  --render        Pipeline completo hasta .png\n";
    std::cerr << "  --ont           Emitir .ont + .obs (formato binario ontológico)\n";
    std::cerr << "  --frames N-M    Cortes en W (defecto: 0..W-1 si W>1)\n";
    std::cerr << "\nEjemplos:\n";
    std::cerr << "  herm escena.herm\n";
    std::cerr << "  herm escena.herm --frames 0-29\n";
    std::cerr << "  herm escena.herm --frames 0-29 -o fotograma_\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage();
        return 1;
    }

    std::string input_path = argv[1];
    std::string output_prefix;
    bool binary_output = false;
    bool emit_tb = false;
    bool emit_gte = false;
    bool render = false;
    bool emit_ont = false;
    bool has_frames = false;
    int frame_start = 0;
    int frame_end = 0;

    for (int i = 2; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-o" && i + 1 < argc) output_prefix = argv[++i];
        else if (arg == "--bin") binary_output = true;
        else if (arg == "--tb") emit_tb = true;
        else if (arg == "--gte") emit_gte = true;
        else if (arg == "--render") render = true;
        else if (arg == "--ont") emit_ont = true;
        else if (arg == "--frames" && i + 1 < argc) {
            std::string range = argv[++i];
            auto dash = range.find('-');
            if (dash != std::string::npos) {
                frame_start = std::stoul(range.substr(0, dash));
                frame_end = std::stoul(range.substr(dash + 1));
                has_frames = true;
            }
        } else {
            std::cerr << "Error: opcion desconocida '" << arg << "'\n\n";
            printUsage();
            return 1;
        }
    }

    try {
        // Read input
        std::ifstream file(input_path);
        if (!file) {
            std::cerr << "Error: no se puede abrir '" << input_path << "'\n";
            return 1;
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string source = buffer.str();

        auto start = std::chrono::high_resolution_clock::now();

        // Lex
        herm::Lexer lexer(source);
        auto tokens = lexer.tokenize();

        for (auto& t : tokens) {
            if (t.type == herm::TokenType::ERROR) {
                std::cerr << "Error lexico: caracter invalido '" << t.value
                          << "' (L" << t.line << ":" << t.col << ")\n";
                return 1;
            }
        }

        // Build file loader for imports
        auto source_dir = fs::path(input_path).parent_path();
        auto cwd = fs::current_path();
        std::vector<fs::path> search_paths;
        search_paths.push_back(source_dir.empty() ? cwd : source_dir);
        search_paths.push_back(cwd / "libreria");
        // Search in .gitignored library (standard library lives there)
        {
            fs::path gl = R"(D:\documentos\Jonatan Baron\proyectos-index\proyecto de IA artistica\Lenguaje Hermetico\.gitignored\libreria)";
            if (fs::exists(gl)) search_paths.push_back(gl);
        }

        herm::FileLoader loader = [&](const std::string& path) -> std::string {
            for (auto& base : search_paths) {
                auto full = base / path;
                std::ifstream f(full);
                if (f) {
                    std::stringstream buf;
                    buf << f.rdbuf();
                    return buf.str();
                }
            }
            return {};
        };

        // Parse
        herm::Parser parser(tokens, loader);
        auto ast = parser.parse();

            // Resolve
        herm::Resolver resolver(ast);
        auto rih = resolver.resolve();

        // 2.0c.1: el .ont no puede representar parametros SDF que dependan de
        // x/y/z. evaluateFrame() los plegaria con x=y=z=0 y la GPU dibujaria
        // otra cosa. Fallar antes de escribir cualquier archivo.
        if (emit_ont) herm::validateOntCompatibility(rih);

        // DL-32: --render/--gte no pueden evaluar por muestra. Fallar antes de
        // escribir el PNG/GTE en vez de dibujar params[i].constant (0).
        if (render || emit_gte) herm::validateRenderCompatibility(rih);

        auto parse_end = std::chrono::high_resolution_clock::now();
        auto parse_ms = std::chrono::duration_cast<std::chrono::milliseconds>(parse_end - start).count();

        // Auto-frame: si W > 1 y no se especificó --frames, barrer todo W
        if (!has_frames && rih.scene.w_frames > 1) {
            frame_start = 0;
            frame_end = (int)rih.scene.w_frames - 1;
            has_frames = true;
        }

        // Derive output prefix if not given
        if (output_prefix.empty()) {
            output_prefix = input_path;
            auto dot = output_prefix.rfind('.');
            if (dot != std::string::npos) output_prefix = output_prefix.substr(0, dot);
            if (has_frames) output_prefix += "_";
        }

        // Track total frames and time
        int frame_count = 0;
        auto emit_start = std::chrono::high_resolution_clock::now();

        if (has_frames) {
            // Frame loop — barrer cortes en W
            for (int f = frame_start; f <= frame_end; f++) {
                auto frame_rih = herm::evaluateFrame(rih, (float)f);
                auto out_path = herm::framePath(output_prefix, f, binary_output);

                bool ok = false;
                if (render) {
                    auto dot = out_path.rfind('.');
                    auto png_path = (dot != std::string::npos) ? out_path.substr(0, dot) : out_path;
                    png_path += ".png";
                    auto pixels = herm::renderRih(frame_rih, (int)frame_rih.scene.width, (int)frame_rih.scene.height);
                    ok = herm::writePng(pixels, (int)frame_rih.scene.width, (int)frame_rih.scene.height, png_path);
                    if (ok) std::cout << "  " << png_path << "\n";
                    else std::cerr << "Error: no se pudo escribir '" << png_path << "'\n";
                } else if (emit_gte) {
                    auto gte_path = herm::framePath(output_prefix, f, false);
                    auto dot = gte_path.rfind('.');
                    if (dot != std::string::npos) gte_path = gte_path.substr(0, dot);
                    gte_path += ".gte";
                    auto pixels = herm::renderRih(frame_rih, (int)frame_rih.scene.width, (int)frame_rih.scene.height);
                    ok = herm::writeGte(frame_rih, pixels, (int)frame_rih.scene.width, (int)frame_rih.scene.height, gte_path);
                    if (ok) std::cout << "  " << gte_path << "\n";
                    else std::cerr << "Error: no se pudo escribir '" << gte_path << "'\n";
                } else {
                    ok = binary_output
                        ? herm::writeRihBinary(frame_rih, out_path)
                        : herm::writeRihJson(frame_rih, out_path);
                if (ok) std::cout << "  " << out_path << "\n";
                else std::cerr << "Error: no se pudo escribir '" << out_path << "'\n";

                // Emit .ont + .obs alongside .rih
                if (emit_ont) {
                    auto slash2 = out_path.find_last_of("/\\");
                    auto dot2 = out_path.rfind('.');
                    if (dot2 == std::string::npos || (slash2 != std::string::npos && dot2 < slash2))
                        dot2 = std::string::npos;
                    auto ont_path = (dot2 != std::string::npos) ? out_path.substr(0, dot2) : out_path;
                    ont_path += ".ont";
                    // 2.0c.3b (DL-36): las expresiones de parametro SDF llegan
                    // vivas a writeOnt; solo se pliegan las transformaciones.
                    auto ont_rih = herm::evaluateFrame(rih, (float)f, false);
                    bool ont_ok = herm::writeOnt(ont_rih, ont_path);
                    if (ont_ok) {
                        std::cout << "  " << ont_path << "\n";
                        herm::writeObs(ont_rih, ont_path);
                        std::cout << "  " << ont_path.substr(0, ont_path.rfind('.')) << ".obs\n";
                    } else {
                        std::cerr << "Error: no se pudo escribir '" << ont_path << "'\n";
                    }
                }
                }

                if (ok) frame_count++;
            }

            auto emit_end = std::chrono::high_resolution_clock::now();
            auto emit_ms = std::chrono::duration_cast<std::chrono::milliseconds>(emit_end - emit_start).count();

            std::cout << "OK " << frame_count << " fotogramas"
                      << " (" << rih.materials.size() << " materiales, "
                      << rih.nodes.size() << " nodos, "
                      << parse_ms << "ms parse + " << emit_ms << "ms eval+emit)\n";

        } else {
            // Single frame
            if (render) {
                auto png_path = output_prefix;
                auto dot = png_path.rfind('.');
                if (dot != std::string::npos) png_path = png_path.substr(0, dot);
                png_path += ".png";

                // DL-32: plegar w/constantes antes de renderizar. Sin esto,
                // renderRih lee params[i].constant (0) y dibuja otra cosa.
                auto frame_rih = herm::evaluateFrame(rih, 0.0f);
                auto pixels = herm::renderRih(frame_rih, (int)frame_rih.scene.width, (int)frame_rih.scene.height);
                bool ok = herm::writePng(pixels, (int)rih.scene.width, (int)rih.scene.height, png_path);

                auto emit_end = std::chrono::high_resolution_clock::now();
                auto total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(emit_end - start).count();

                if (ok) {
                    std::cout << "OK " << png_path
                              << " (" << rih.materials.size() << " materiales, "
                              << rih.nodes.size() << " nodos, "
                              << parse_ms << "ms parse + " << (total_ms - parse_ms) << "ms render+emit)\n";
                } else {
                    std::cerr << "Error: no se pudo escribir '" << png_path << "'\n";
                    return 1;
                }
            } else if (emit_gte) {
                auto gte_path = output_prefix;
                auto dot = gte_path.rfind('.');
                if (dot != std::string::npos) gte_path = gte_path.substr(0, dot);
                gte_path += ".gte";

                auto frame_rih = herm::evaluateFrame(rih, 0.0f);
                auto pixels = herm::renderRih(frame_rih, (int)frame_rih.scene.width, (int)frame_rih.scene.height);
                bool ok = herm::writeGte(frame_rih, pixels, (int)frame_rih.scene.width, (int)frame_rih.scene.height, gte_path);

                auto emit_end = std::chrono::high_resolution_clock::now();
                auto total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(emit_end - start).count();

                if (ok) {
                    std::cout << "OK " << gte_path
                              << " (" << rih.materials.size() << " materiales, "
                              << rih.nodes.size() << " nodos, "
                              << parse_ms << "ms parse + " << (total_ms - parse_ms) << "ms render+emit)\n";
                } else {
                    std::cerr << "Error: no se pudo escribir '" << gte_path << "'\n";
                    return 1;
                }
            } else {
                if (output_prefix.find('.') == std::string::npos)
                    output_prefix += binary_output ? ".rib" : ".rih";

                bool ok = binary_output
                    ? herm::writeRihBinary(rih, output_prefix)
                    : herm::writeRihJson(rih, output_prefix);

                auto emit_end = std::chrono::high_resolution_clock::now();
                auto total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(emit_end - start).count();

                if (ok) {
                    std::cout << "OK " << output_prefix
                              << " (" << rih.materials.size() << " materiales, "
                              << rih.nodes.size() << " nodos, "
                              << parse_ms << "ms parse + " << (total_ms - parse_ms) << "ms emit)\n";
                    if (emit_ont) {
                        auto slash = output_prefix.find_last_of("/\\");
                        auto dot3 = output_prefix.rfind('.');
                        // Only use dot if it's after the last directory separator
                        if (dot3 == std::string::npos || (slash != std::string::npos && dot3 < slash))
                            dot3 = std::string::npos;
                        auto ont_path = (dot3 != std::string::npos) ? output_prefix.substr(0, dot3) : output_prefix;
                        ont_path += ".ont";
                        // W=1: no hay bucle de fotogramas. evaluateFrame(.,0,false)
                        // conserva las expresiones de parametro SDF para que
                        // writeOnt las compile (2.0c.3b, DL-36).
                        auto ont_rih = herm::evaluateFrame(rih, 0.0f, false);
                        bool ont_ok = herm::writeOnt(ont_rih, ont_path);
                        if (ont_ok) {
                            herm::writeObs(ont_rih, ont_path);
                            std::cout << "  " << ont_path << "\n";
                        } else {
                            std::cerr << "Error: no se pudo escribir '" << ont_path << "'\n";
                        }
                    }
                } else {
                    std::cerr << "Error: no se pudo escribir '" << output_prefix << "'\n";
                    return 1;
                }
            }
        }

    } catch (const herm::ParseError& e) {
        std::cerr << "Error de sintaxis: " << e.what() << "\n";
        return 1;
    } catch (const herm::ResolverError& e) {
        std::cerr << "Error semantico: " << e.what() << "\n";
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
