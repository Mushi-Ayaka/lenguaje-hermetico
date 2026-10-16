// herm_compile.cpp - API de compilacion en memoria del Lenguaje Hermético.
// Implementa contrato/herm.h (compile, compileFromString) y compileToRih,
// que usa core/herm_bridge.cpp del motor gráfico para live-compile sin disco.
//
// Pipeline: Lexer -> tokenize -> Parser -> AST -> Resolver -> Rih.
#include "herm_lexer.h"
#include "herm_parser.h"
#include "herm_resolver.h"
#include "herm_rih.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>

// herm.h usa f64 pero rih.h no lo define; debe ir ANTES del include.
namespace herm { using f64 = double; }
#include "../contrato/herm.h"

namespace fs = std::filesystem;

namespace herm {

// ---------------------------------------------------------------------------
// Loader por defecto para `import`: misma búsqueda que main.cpp.
// (compileToRih trabaja sobre un string, no hay source_dir propio.)
// ---------------------------------------------------------------------------
static FileLoader defaultLoader() {
    return [](const std::string& path) -> std::string {
        std::vector<fs::path> search_paths;
        search_paths.push_back(fs::current_path());
        search_paths.push_back(fs::current_path() / "libreria");
        {
            fs::path gl = R"(D:\documentos\Jonatan Baron\proyectos-index\proyecto de IA artistica\Lenguaje Hermetico\.gitignored\libreria)";
            if (fs::exists(gl)) search_paths.push_back(gl);
        }
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
}

// ---------------------------------------------------------------------------
// compileToRih - compila source en memoria y devuelve el Rih sin escribir disco.
// errOut (opcional) recibe el mensaje de error en caso de fallo.
// ---------------------------------------------------------------------------
bool compileToRih(const std::string& source, Rih& out, std::string* errOut) {
    try {
        Lexer lexer(source);
        auto tokens = lexer.tokenize();

        for (auto& t : tokens) {
            if (t.type == TokenType::ERROR) {
                if (errOut) {
                    std::ostringstream os;
                    os << "error lexico: caracter invalido '" << t.value
                       << "' (L" << t.line << ":" << t.col << ")";
                    *errOut = os.str();
                }
                return false;
            }
        }

        Parser parser(tokens, defaultLoader());
        auto ast = parser.parse();      // puede lanzar ParseError

        Resolver resolver(ast);
        out = resolver.resolve();       // puede lanzar ResolverError
        return true;
    } catch (const std::exception& e) {
        if (errOut) *errOut = e.what();
        return false;
    }
}

// ---------------------------------------------------------------------------
// compileFromString - contrato/herm.h. Compila y (si config.output_path tiene
// valor) escribe .rih/.rib segun config.binary_output.
// ---------------------------------------------------------------------------
HermResult compileFromString(const std::string& source, const HermConfig& config) {
    HermResult result{};
    auto t0 = std::chrono::high_resolution_clock::now();

    Rih rih;
    std::string err;
    if (!compileToRih(source, rih, &err)) {
        result.success = false;
        result.errors.push_back({0, 0, err, ""});
        return result;
    }

    if (!config.output_path.empty()) {
        bool ok = config.binary_output
            ? writeRihBinary(rih, config.output_path)
            : writeRihJson(rih, config.output_path);
        if (!ok) {
            result.success = false;
            result.errors.push_back({0, 0,
                "no se pudo escribir '" + config.output_path + "'", ""});
            return result;
        }
        result.output_path = config.output_path;
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    result.elapsed_ms = std::chrono::duration<f64, std::milli>(t1 - t0).count();
    result.success = true;
    return result;
}

// ---------------------------------------------------------------------------
// compile - contrato/herm.h. Lee config.input_path de disco y compila.
// ---------------------------------------------------------------------------
HermResult compile(const HermConfig& config) {
    std::ifstream file(config.input_path);
    if (!file) {
        HermResult result{};
        result.success = false;
        result.errors.push_back({0, 0,
            "no se puede abrir '" + config.input_path + "'", ""});
        return result;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();

    HermConfig cfg = config;
    if (cfg.output_path.empty()) {
        // Derivar salida: mismo path con extensión .rih (o .rib si binario).
        cfg.output_path = config.input_path;
        auto dot = cfg.output_path.rfind('.');
        if (dot != std::string::npos) cfg.output_path = cfg.output_path.substr(0, dot);
        cfg.output_path += cfg.binary_output ? ".rib" : ".rih";
    }
    return compileFromString(buffer.str(), cfg);
}

} // namespace herm
