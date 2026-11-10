#include "herm_eval.h"
#include "contrato/ont.h"
#include <cmath>
#include <cctype>
#include <stdexcept>
#include <string>

namespace herm {

// ============================================================================
// Simple recursive-descent expression evaluator
// Supports: + - * / % ^ sin cos tan tanh abs sqrt floor ceil
//           exp log log2 log10 acos asin atan atan2 sign mod fract
//           min max pow clamp lerp mix step smoothstep sample
//           variable t (x,y,z,w), shading vars (nx..lz)
//           constants pi/e/LN2/PLANCK
// Fallos: funcion o variable desconocida, parentesis sin cerrar y
//         expresion invalida lanzan std::runtime_error (Paso 2.0b).
//         No hay fallback silencioso a 0.0f.
// ============================================================================

class ExprParser {
public:
    ExprParser(const std::string& s, float t)
        : s(s), x(0), y(0), z(0), w(t),
          nx(0), ny(0), nz(0),
          vx(0), vy(0), vz(0),
          lx(0), ly(0), lz(0),
          depth(0), pos(0) {}

    ExprParser(const std::string& s, float x, float y, float z, float w,
               float nx, float ny, float nz,
               float vx, float vy, float vz,
               float lx, float ly, float lz,
               int depth)
        : s(s), x(x), y(y), z(z), w(w),
          nx(nx), ny(ny), nz(nz),
          vx(vx), vy(vy), vz(vz),
          lx(lx), ly(ly), lz(lz),
          depth(depth), pos(0) {}

    float parse() {
        skipWs();
        float r = expr();
        skipWs();
        if (pos < s.size())
            throw std::runtime_error("expresion invalida en '" + s + "' (pos " + std::to_string(pos) + ")");
        return r;
    }

private:
    const std::string& s;
    float x, y, z, w;
    float nx, ny, nz;
    float vx, vy, vz;
    float lx, ly, lz;
    int depth;
    size_t pos;

    char peek() const { return pos < s.size() ? s[pos] : '\0'; }
    char advance() { return pos < s.size() ? s[pos++] : '\0'; }

    void skipWs() {
        while (pos < s.size() && std::isspace(s[pos])) pos++;
    }

    // expr = term (('+'|'-') term)*
    float expr() {
        float r = term();
        skipWs();
        while (peek() == '+' || peek() == '-') {
            char op = advance();
            skipWs();
            float rhs = term();
            r = (op == '+') ? r + rhs : r - rhs;
        }
        return r;
    }

    // term = unary (('*'|'/'|'%') unary)*
    float term() {
        float r = unary();
        skipWs();
        while (peek() == '*' || peek() == '/' || peek() == '%') {
            char op = advance();
            skipWs();
            float rhs = unary();
            if (op == '*') r *= rhs;
            else if (op == '/') r /= rhs;   // IEEE: inf/NaN como en GPU, sin fallback a 0
            else r = std::fmod(r, rhs);
        }
        return r;
    }

    float unary() {
        skipWs();
        if (peek() == '-') { advance(); return -unary(); }
        if (peek() == '+') { advance(); return unary(); }
        return power();
    }

    // power = primary ('^' primary)*
    float power() {
        float r = primary();
        skipWs();
        if (peek() == '^') {
            advance();
            skipWs();
            float rhs = primary();
            r = std::pow(r, rhs);
        }
        return r;
    }

    float primary() {
        skipWs();
        char c = peek();

        // Number
        if (c == '.' || std::isdigit(c) || (c == '-' && pos + 1 < s.size() && std::isdigit(s[pos + 1])))
            return number();

        // '(' expr ')'
        if (c == '(') {
            advance();
            float r = expr();
            skipWs();
            if (peek() == ')') advance();
            return r;
        }

        // Identifier: function call, variable, or constant
        if (std::isalpha(c) || c == '_') {
            std::string name;
            while (pos < s.size() && (std::isalnum(s[pos]) || s[pos] == '_'))
                name += advance();

            // Constants
            if (name == "pi") return 3.14159265358979323846f;
            if (name == "e")  return 2.71828182845904523536f;
            if (name == "LN2") return 0.69314718055994530942f;
            if (name == "PLANCK") return 6.62607015e-34f;

            // Variables
            if (name == "x") return x;
            if (name == "y") return y;
            if (name == "z") return z;
            if (name == "w" || name == "t") return w;

            // Shading variables
            if (name == "nx") return nx;
            if (name == "ny") return ny;
            if (name == "nz") return nz;
            if (name == "vx") return vx;
            if (name == "vy") return vy;
            if (name == "vz") return vz;
            if (name == "lx") return lx;
            if (name == "ly") return ly;
            if (name == "lz") return lz;

            // Functions — look for optional '(' after whitespace
            skipWs();
            bool hasParen = (peek() == '(');
            if (hasParen) advance(); // consume '('

            auto parseArg = [&]() -> float { skipWs(); return expr(); };
            auto closeParen = [&]() {
                skipWs();
                if (hasParen) {
                    if (peek() != ')')
                        throw std::runtime_error("falta ')' en '" + s + "'");
                    advance();
                }
            };

            float arg, arg2, arg3;

            // One-arg functions
            if (name == "sin")   { arg = parseArg(); closeParen(); return std::sin(arg); }
            if (name == "cos")   { arg = parseArg(); closeParen(); return std::cos(arg); }
            if (name == "tan")   { arg = parseArg(); closeParen(); return std::tan(arg); }
            if (name == "tanh")  { arg = parseArg(); closeParen(); return std::tanh(arg); }
            if (name == "abs")   { arg = parseArg(); closeParen(); return std::abs(arg); }
            if (name == "sqrt")  { arg = parseArg(); closeParen(); return std::sqrt(arg); }
            if (name == "floor") { arg = parseArg(); closeParen(); return std::floor(arg); }
            if (name == "ceil")  { arg = parseArg(); closeParen(); return std::ceil(arg); }
            if (name == "exp")   { arg = parseArg(); closeParen(); return std::exp(arg); }
            if (name == "log")   { arg = parseArg(); closeParen(); return std::log(arg); }
            if (name == "log2")  { arg = parseArg(); closeParen(); return std::log2(arg); }
            if (name == "log10") { arg = parseArg(); closeParen(); return std::log10(arg); }
            if (name == "acos")  { arg = parseArg(); closeParen(); return std::acos(arg); }
            if (name == "asin")  { arg = parseArg(); closeParen(); return std::asin(arg); }
            if (name == "atan")  { arg = parseArg(); closeParen(); return std::atan(arg); }
            if (name == "sign")  { arg = parseArg(); closeParen();
                                   return (arg > 0.0f) ? 1.0f : ((arg < 0.0f) ? -1.0f : 0.0f); }
            if (name == "fract") { arg = parseArg(); closeParen(); return arg - std::floor(arg); }

            // Two-arg functions
            if (name == "pow" || name == "min" || name == "max" ||
                name == "atan2" || name == "mod" || name == "step") {
                arg = parseArg();
                skipWs(); if (peek() == ',') advance();
                arg2 = parseArg();
                closeParen();
                if (name == "min")   return std::fmin(arg, arg2);
                if (name == "max")   return std::fmax(arg, arg2);
                if (name == "pow")   return std::pow(arg, arg2);
                if (name == "atan2") return std::atan2(arg, arg2);
                // GLSL mod: x - y*floor(x/y), no fmod (difiere en negativos)
                if (name == "mod")   return arg - arg2 * std::floor(arg / arg2);
                // GLSL step(edge, x): 0 si x < edge, 1 si x >= edge
                if (name == "step")  return (arg2 < arg) ? 0.0f : 1.0f;
            }

            // Three-arg functions
            if (name == "clamp" || name == "lerp" || name == "mix" ||
                name == "sample" || name == "smoothstep") {
                arg = parseArg();
                skipWs(); if (peek() == ',') advance();
                arg2 = parseArg();
                skipWs(); if (peek() == ',') advance();
                arg3 = parseArg();
                closeParen();
                if (name == "clamp") return std::fmax(std::fmin(arg, arg3), arg2);
                if (name == "lerp" || name == "mix") return arg + (arg2 - arg) * arg3;
                if (name == "smoothstep") {
                    float t = (arg3 - arg) / (arg2 - arg);
                    t = std::fmax(0.0f, std::fmin(1.0f, t));
                    return t * t * (3.0f - 2.0f * t);
                }
                if (name == "sample") {
                    if (depth >= 1)
                        throw std::runtime_error("sample anidado (profundidad > 1) en '" + s + "'");
                    return evalExpression(s, x + arg, y + arg2, z + arg3, w,
                                          nx, ny, nz, vx, vy, vz, lx, ly, lz, depth + 1);
                }
            }

            // Desconocido: fallo ruidoso. Antes devolvia 0.0f en silencio (Paso 2.0b).
            throw std::runtime_error("funcion o variable desconocida '" + name + "' en '" + s + "'");
        }

        throw std::runtime_error("caracter inesperado '" + std::string(1, c) + "' en '" + s + "'");
    }

    float number() {
        skipWs();
        std::string val;
        if (peek() == '-') { val += advance(); }
        while (pos < s.size() && std::isdigit(s[pos])) val += advance();
        if (peek() == '.') {
            val += advance();
            while (pos < s.size() && std::isdigit(s[pos])) val += advance();
        }
        if (peek() == 'e' || peek() == 'E') {
            val += advance();
            if (peek() == '+' || peek() == '-') val += advance();
            while (pos < s.size() && std::isdigit(s[pos])) val += advance();
        }
        return val.empty() ? 0.0f : std::stof(val);
    }
};

// ============================================================================
// 2.0c.3b (DL-36): expression -> ONT postfix bytecode
//
// Replica la gramatica de ExprParser (misma precedencia, mismo `^` no
// asociativo, mismas llamadas sin parentesis). La diferencia es que en vez
// de calcular emite instrucciones de 5 bytes. Cualquier cambio de gramatica
// debe hacerse en los dos sitios; la prueba diferencial lo detecta.
// ============================================================================

class OntExprCompiler {
public:
    OntExprCompiler(const std::string& s, OntExprSink& sink)
        : s(s), sink(sink), pos(0) {}

    void parse() {
        skipWs();
        expr();
        skipWs();
        if (pos < s.size())
            throw std::runtime_error("expresion invalida en '" + s + "' (pos " +
                                     std::to_string(pos) + ")");
    }

private:
    const std::string& s;
    OntExprSink& sink;
    size_t pos;

    char peek() const { return pos < s.size() ? s[pos] : '\0'; }
    char advance() { return pos < s.size() ? s[pos++] : '\0'; }

    void skipWs() {
        while (pos < s.size() && std::isspace((unsigned char)s[pos])) pos++;
    }

    void expr() {
        term();
        skipWs();
        while (peek() == '+' || peek() == '-') {
            char op = advance();
            skipWs();
            term();
            sink.emitOp(op == '+' ? ONT_ADD : ONT_SUB);
        }
    }

    void term() {
        unary();
        skipWs();
        while (peek() == '*' || peek() == '/' || peek() == '%') {
            char op = advance();
            skipWs();
            if (op == '%')
                throw std::runtime_error(
                    "el operador '%' usa fmod y no tiene opcode ONT; "
                    "usa mod(a,b) en '" + s + "'");
            unary();
            sink.emitOp(op == '*' ? ONT_MUL : ONT_DIV);
        }
    }

    void unary() {
        skipWs();
        if (peek() == '-') {
            advance();
            unary();
            sink.emitOp(ONT_NEG);
            return;
        }
        if (peek() == '+') {
            advance();
            unary();
            return;
        }
        power();
    }

    void power() {
        primary();
        skipWs();
        if (peek() == '^') {
            advance();
            skipWs();
            primary();
            sink.emitOp(ONT_POW);
        }
    }

    void primary() {
        skipWs();
        char c = peek();

        // Numero (incluye el '-' de 2^-3, igual que ExprParser::number)
        if (c == '.' || std::isdigit((unsigned char)c) ||
            (c == '-' && pos + 1 < s.size() &&
             std::isdigit((unsigned char)s[pos + 1]))) {
            sink.emitConst(number());
            return;
        }

        // '(' expr ')'
        if (c == '(') {
            advance();
            expr();
            skipWs();
            if (peek() == ')') advance();
            return;
        }

        // Identificador: constante, variable o funcion
        if (std::isalpha((unsigned char)c) || c == '_') {
            std::string name;
            while (pos < s.size() &&
                   (std::isalnum((unsigned char)s[pos]) || s[pos] == '_'))
                name += advance();

            // Constantes
            if (name == "pi")     { sink.emitConst(3.14159265358979323846f); return; }
            if (name == "e")      { sink.emitConst(2.71828182845904523536f); return; }
            if (name == "LN2")    { sink.emitConst(0.69314718055994530942f); return; }
            if (name == "PLANCK") { sink.emitConst(6.62607015e-34f); return; }

            // Variables
            if (name == "x") { sink.emitOp(ONT_VAR_X); return; }
            if (name == "y") { sink.emitOp(ONT_VAR_Y); return; }
            if (name == "z") { sink.emitOp(ONT_VAR_Z); return; }
            if (name == "w" || name == "t") { sink.emitOp(ONT_VAR_W); return; }

            // Variables de sombreado: el bytecode tiene opcodes, pero un
            // parametro SDF no tiene contexto de sombreado y los
            // decompiladores GPU no las representan igual. Fallo ruidoso.
            if (name == "nx" || name == "ny" || name == "nz" ||
                name == "vx" || name == "vy" || name == "vz" ||
                name == "lx" || name == "ly" || name == "lz") {
                throw std::runtime_error(
                    "la variable de sombreado '" + name +
                    "' no tiene contexto en un parametro SDF ('" + s + "')");
            }

            // Funciones sin opcode: rechazo explicito antes de parsear args.
            if (name == "tanh")
                throw std::runtime_error(
                    "'tanh' no tiene opcode ONT (los 12 de 2.0c.3b no la "
                    "incluyen) en '" + s + "'");
            if (name == "sample")
                throw std::runtime_error(
                    "'sample' no se puede representar en bytecode ONT "
                    "(reentra en el evaluador con un offset) en '" + s + "'");

            // Parentesis opcional, igual que ExprParser.
            skipWs();
            bool hasParen = (peek() == '(');
            if (hasParen) advance();

            auto parseArg = [&]() { skipWs(); expr(); };
            auto closeParen = [&]() {
                skipWs();
                if (hasParen) {
                    if (peek() != ')')
                        throw std::runtime_error("falta ')' en '" + s + "'");
                    advance();
                }
            };

            // Una entrada
            uint8_t op1 = oneArgOp(name);
            if (op1 != 0xFF) {
                parseArg();
                closeParen();
                sink.emitOp(op1);
                return;
            }

            // Dos entradas
            uint8_t op2 = twoArgOp(name);
            if (op2 != 0xFF) {
                parseArg();
                skipWs(); if (peek() == ',') advance();
                parseArg();
                closeParen();
                sink.emitOp(op2);
                return;
            }

            // Tres entradas
            uint8_t op3 = threeArgOp(name);
            if (op3 != 0xFF) {
                parseArg();
                skipWs(); if (peek() == ',') advance();
                parseArg();
                skipWs(); if (peek() == ',') advance();
                parseArg();
                closeParen();
                sink.emitOp(op3);
                return;
            }

            throw std::runtime_error("funcion o variable desconocida '" +
                                     name + "' en '" + s + "'");
        }

        throw std::runtime_error("caracter inesperado '" +
                                 std::string(1, c) + "' en '" + s + "'");
    }

    static uint8_t oneArgOp(const std::string& name) {
        if (name == "sin")   return ONT_SIN;
        if (name == "cos")   return ONT_COS;
        if (name == "tan")   return ONT_TAN;
        if (name == "abs")   return ONT_ABS;
        if (name == "sqrt")  return ONT_SQRT;
        if (name == "floor") return ONT_FLOOR;
        if (name == "ceil")  return ONT_CEIL;
        if (name == "exp")   return ONT_EXP;
        if (name == "log")   return ONT_LOG;
        if (name == "log2")  return ONT_LOG2;
        if (name == "log10") return ONT_LOG10;
        if (name == "acos")  return ONT_ACOS;
        if (name == "asin")  return ONT_ASIN;
        if (name == "atan")  return ONT_ATAN;
        if (name == "sign")  return ONT_SIGN;
        if (name == "fract") return ONT_FRACT;
        return 0xFF;
    }

    static uint8_t twoArgOp(const std::string& name) {
        if (name == "pow")   return ONT_POW;
        if (name == "min")   return ONT_MIN;
        if (name == "max")   return ONT_MAX;
        if (name == "atan2") return ONT_ATAN2;
        if (name == "mod")   return ONT_MOD;
        if (name == "step")  return ONT_STEP;
        return 0xFF;
    }

    static uint8_t threeArgOp(const std::string& name) {
        if (name == "clamp")      return ONT_CLAMP;
        if (name == "lerp")       return ONT_LERP;
        if (name == "mix")        return ONT_MIX;
        if (name == "smoothstep") return ONT_SMOOTHSTEP;
        return 0xFF;
    }

    float number() {
        skipWs();
        std::string val;
        if (peek() == '-') val += advance();
        while (pos < s.size() && std::isdigit((unsigned char)s[pos]))
            val += advance();
        if (peek() == '.') {
            val += advance();
            while (pos < s.size() && std::isdigit((unsigned char)s[pos]))
                val += advance();
        }
        if (peek() == 'e' || peek() == 'E') {
            val += advance();
            if (peek() == '+' || peek() == '-') val += advance();
            while (pos < s.size() && std::isdigit((unsigned char)s[pos]))
                val += advance();
        }
        try {
            return std::stof(val);
        } catch (...) {
            throw std::runtime_error("numero invalido '" + val + "' en '" + s + "'");
        }
    }
};

void compileOntExpression(const std::string& expr, OntExprSink& sink) {
    if (expr.empty())
        throw std::runtime_error("expresion ONT vacia");
    OntExprCompiler compiler(expr, sink);
    compiler.parse();
}

float evalExpression(const std::string& expr, float t) {
    if (expr.empty()) return 0.0f;
    ExprParser parser(expr, t);
    return parser.parse();
}

float evalExpression(const std::string& expr, float x, float y, float z, float w,
                     float nx, float ny, float nz,
                     float vx, float vy, float vz,
                     float lx, float ly, float lz,
                     int depth) {
    if (expr.empty()) return 0.0f;
    ExprParser parser(expr, x, y, z, w, nx, ny, nz, vx, vy, vz, lx, ly, lz, depth);
    return parser.parse();
}

} // namespace herm
