#include "herm_eval.h"
#include <cmath>
#include <cctype>
#include <stdexcept>
#include <string>

namespace herm {

// ============================================================================
// Simple recursive-descent expression evaluator
// Supports: + - * / % ^ sin cos tan abs sqrt floor ceil
//           min max pow clamp lerp mix, variable t, constants pi/e
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
            else if (op == '/') r = (rhs != 0) ? r / rhs : 0.0f;
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

            float arg, arg2, arg3;

            if (name == "sin")   { arg = parseArg(); if (hasParen && peek() == ')') advance(); return std::sin(arg); }
            if (name == "cos")   { arg = parseArg(); if (hasParen && peek() == ')') advance(); return std::cos(arg); }
            if (name == "tan")   { arg = parseArg(); if (hasParen && peek() == ')') advance(); return std::tan(arg); }
            if (name == "abs")   { arg = parseArg(); if (hasParen && peek() == ')') advance(); return std::abs(arg); }
            if (name == "sqrt")  { arg = parseArg(); if (hasParen && peek() == ')') advance(); return std::sqrt(arg); }
            if (name == "floor") { arg = parseArg(); if (hasParen && peek() == ')') advance(); return std::floor(arg); }
            if (name == "ceil")  { arg = parseArg(); if (hasParen && peek() == ')') advance(); return std::ceil(arg); }

            // Two-arg functions
            if (name == "pow" || name == "min" || name == "max") {
                arg = parseArg();
                skipWs(); if (peek() == ',') advance();
                arg2 = parseArg();
                if (hasParen && peek() == ')') advance();
                if (name == "min") return std::fmin(arg, arg2);
                if (name == "max") return std::fmax(arg, arg2);
                if (name == "pow") return std::pow(arg, arg2);
            }

            // Three-arg functions
            if (name == "clamp" || name == "lerp" || name == "mix" || name == "sample") {
                arg = parseArg();
                skipWs(); if (peek() == ',') advance();
                arg2 = parseArg();
                skipWs(); if (peek() == ',') advance();
                arg3 = parseArg();
                if (hasParen && peek() == ')') advance();
                if (name == "clamp") return std::fmax(std::fmin(arg3, arg2), arg);
                if (name == "lerp" || name == "mix") return arg + (arg2 - arg) * arg3;
                if (name == "sample") {
                    if (depth >= 1) return 0.0f;
                    return evalExpression(s, x + arg, y + arg2, z + arg3, w,
                                          nx, ny, nz, vx, vy, vz, lx, ly, lz, depth + 1);
                }
            }

            // Unknown — try to consume any remaining ')'
            if (hasParen) { while (peek() != ')' && pos < s.size()) advance(); if (peek() == ')') advance(); }
            return 0.0f;
        }

        return 0.0f;
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

float evalExpression(const std::string& expr, float t) {
    if (expr.empty()) return 0.0f;
    try {
        ExprParser parser(expr, t);
        return parser.parse();
    } catch (...) {
        return 0.0f;
    }
}

float evalExpression(const std::string& expr, float x, float y, float z, float w,
                     float nx, float ny, float nz,
                     float vx, float vy, float vz,
                     float lx, float ly, float lz,
                     int depth) {
    if (expr.empty()) return 0.0f;
    try {
        ExprParser parser(expr, x, y, z, w, nx, ny, nz, vx, vy, vz, lx, ly, lz, depth);
        return parser.parse();
    } catch (...) {
        return 0.0f;
    }
}

} // namespace herm
