#include "herm_lexer.h"
#include <stdexcept>
#include <cctype>

namespace herm {

const std::unordered_map<std::string, TokenType> Lexer::keywords_en = {
    {"scene", TokenType::SCENE},
    {"axes", TokenType::AXES},
    {"camera", TokenType::CAMERA},
    {"background", TokenType::BACKGROUND},
    {"material", TokenType::MATERIAL},
    {"node", TokenType::NODE},
    {"def", TokenType::DEF},
    {"instance", TokenType::INSTANCE},
    {"sdf", TokenType::SDF},
    {"mode", TokenType::MODE},
    {"solid", TokenType::SOLID},
    {"volume", TokenType::VOLUME},
    {"visible", TokenType::VISIBLE},
    {"true", TokenType::TRUE},
    {"false", TokenType::FALSE},
    {"for", TokenType::FOR},
    {"import", TokenType::IMPORT},
    {"let", TokenType::LET},
    {"fn", TokenType::FN},
    {"light", TokenType::LIGHT},
    {"color", TokenType::COLOR_BASE},
    {"type", TokenType::TYPE},
    {"point", TokenType::POINT_LIGHT},
    {"direction", TokenType::DIRECTION},
    {"intensity", TokenType::INTENSITY},
    {"falloff", TokenType::FALLOFF},

    // Material fields
    {"tensor", TokenType::TENSOR},
    {"base_color", TokenType::BASE_COLOR},
    {"roughness", TokenType::ROUGHNESS},
    {"metallic", TokenType::METALLIC},
    {"emission", TokenType::EMISSION},
    {"emission_mode", TokenType::EMISSION_MODE},
    {"emission_dir", TokenType::EMISSION_DIR},
    {"isotropic", TokenType::ISOTROPIC},
    {"directional", TokenType::DIRECTIONAL},
    {"ior", TokenType::IOR},
    {"transmission", TokenType::TRANSMISSION},
    {"opacity", TokenType::OPACITY},
    {"subsurface", TokenType::SUBSURFACE},
    {"blend_mode", TokenType::BLEND_MODE},
    {"alpha", TokenType::ALPHA},
    {"add", TokenType::ADD},
    {"replace", TokenType::REPLACE},

    // Transform fields
    {"position", TokenType::POSITION},
    {"target", TokenType::TARGET},
    {"up", TokenType::UP},
    {"fov", TokenType::FOV},
    {"aperture", TokenType::APERTURE},
    {"focal_dist", TokenType::FOCAL_DIST},
    {"translate", TokenType::TRANSLATE},
    {"rotation", TokenType::ROTATION},
    {"scale", TokenType::SCALE},
    {"density_scale", TokenType::DENSITY_SCALE},

    // SDF primitives
    {"sphere", TokenType::SPHERE},
    {"box", TokenType::BOX},
    {"cylinder", TokenType::CYLINDER},
    {"capsule", TokenType::CAPSULE},
    {"torus", TokenType::TORUS},
    {"plane", TokenType::PLANE},
    {"cone", TokenType::CONE},
    {"rounded_box", TokenType::ROUNDED_BOX},
    {"octahedron", TokenType::OCTAHEDRON},
    {"tesseract", TokenType::TESSERACT},
    {"sphere4d", TokenType::SPHERE4D},

    // SDF operators
    {"union", TokenType::UNION},
    {"subtract", TokenType::SUBTRACT},
    {"intersect", TokenType::INTERSECT},
    {"smooth_union", TokenType::SMOOTH_UNION},
    {"smooth_subtract", TokenType::SMOOTH_SUBTRACT},
    {"smooth_intersect", TokenType::SMOOTH_INTERSECT},
    {"repeat", TokenType::REPEAT},
    {"twist", TokenType::TWIST},
    {"elongate", TokenType::ELONGATE},
    {"displace", TokenType::DISPLACE},
};

const std::unordered_map<std::string, TokenType> Lexer::keywords_es = {
    {"escena", TokenType::SCENE},
    {"ejes", TokenType::AXES},
    {"camara", TokenType::CAMERA},
    {"fondo", TokenType::BACKGROUND},
    {"material", TokenType::MATERIAL},
    {"nodo", TokenType::NODE},
    {"def", TokenType::DEF},
    {"instancia", TokenType::INSTANCE},
    {"sdf", TokenType::SDF},
    {"modo", TokenType::MODE},
    {"solido", TokenType::SOLID},
    {"volumen", TokenType::VOLUME},
    {"visible", TokenType::VISIBLE},
    {"verdadero", TokenType::TRUE},
    {"falso", TokenType::FALSE},
    {"para", TokenType::FOR},
    {"importar", TokenType::IMPORTAR},
    {"sea", TokenType::SEA},
    {"funcion", TokenType::FUNCION},
    {"lumbre", TokenType::LUMBRE},
    {"tipo", TokenType::TIPO},
    {"puntual", TokenType::PUNTUAL},
    {"direccion", TokenType::DIRECTION},
    {"intensidad", TokenType::INTENSITY},
    {"caida", TokenType::FALLOFF},

    {"tensor", TokenType::TENSOR},
    {"color_base", TokenType::BASE_COLOR},
    {"rugosidad", TokenType::ROUGHNESS},
    {"metalico", TokenType::METALLIC},
    {"emision", TokenType::EMISSION},
    {"modo_emision", TokenType::EMISSION_MODE},
    {"dir_emision", TokenType::EMISSION_DIR},
    {"isotropico", TokenType::ISOTROPIC},
    {"direccional", TokenType::DIRECTIONAL},
    {"ior", TokenType::IOR},
    {"transmision", TokenType::TRANSMISSION},
    {"opacidad", TokenType::OPACITY},
    {"subsuperficie", TokenType::SUBSURFACE},
    {"modo_mezcla", TokenType::BLEND_MODE},
    {"alpha", TokenType::ALPHA},
    {"sumar", TokenType::ADD},
    {"reemplazar", TokenType::REPLACE},

    {"posicion", TokenType::POSITION},
    {"objetivo", TokenType::TARGET},
    {"arriba", TokenType::UP},
    {"fov", TokenType::FOV},
    {"apertura", TokenType::APERTURE},
    {"distancia_focal", TokenType::FOCAL_DIST},
    {"traslacion", TokenType::TRANSLATE},
    {"rotacion", TokenType::ROTATION},
    {"escala", TokenType::SCALE},
    {"escala_densidad", TokenType::DENSITY_SCALE},

    {"esfera", TokenType::SPHERE},
    {"caja", TokenType::BOX},
    {"cilindro", TokenType::CYLINDER},
    {"capsula", TokenType::CAPSULE},
    {"toro", TokenType::TORUS},
    {"plano", TokenType::PLANE},
    {"cono", TokenType::CONE},
    {"caja_redondeada", TokenType::ROUNDED_BOX},
    {"octaedro", TokenType::OCTAHEDRON},
    {"tesseract", TokenType::TESSERACT},
    {"hipercubo", TokenType::HIPERCUBO},
    {"esfera4d", TokenType::SPHERE4D},

    {"union", TokenType::UNION},
    {"resta", TokenType::SUBTRACT},
    {"interseccion", TokenType::INTERSECT},
    {"union_suave", TokenType::SMOOTH_UNION},
    {"resta_suave", TokenType::SMOOTH_SUBTRACT},
    {"interseccion_suave", TokenType::SMOOTH_INTERSECT},
    {"repetir", TokenType::REPEAT},
    {"torsion", TokenType::TWIST},
    {"alargar", TokenType::ELONGATE},
    {"desplazar", TokenType::DISPLACE},
};

TokenType Lexer::lookupKeyword(const std::string& val) {
    auto it_en = keywords_en.find(val);
    if (it_en != keywords_en.end()) return it_en->second;
    auto it_es = keywords_es.find(val);
    if (it_es != keywords_es.end()) return it_es->second;
    return TokenType::IDENT;
}

std::string Lexer::typeName(TokenType t) {
    switch (t) {
        case TokenType::SCENE: return "scene";
        case TokenType::AXES: return "axes";
        case TokenType::CAMERA: return "camera";
        case TokenType::BACKGROUND: return "background";
        case TokenType::MATERIAL: return "material";
        case TokenType::NODE: return "node";
        case TokenType::DEF: return "def";
        case TokenType::INSTANCE: return "instance";
        case TokenType::IMPORT: return "import";
        case TokenType::IMPORTAR: return "importar";
        case TokenType::LET: return "let";
        case TokenType::SEA: return "sea";
        case TokenType::LIGHT: return "light";
        case TokenType::LUMBRE: return "lumbre";
        case TokenType::TYPE: return "type";
        case TokenType::TIPO: return "tipo";
        case TokenType::POINT_LIGHT: return "point";
        case TokenType::PUNTUAL: return "puntual";
        case TokenType::DIRECTION: return "direction";
        case TokenType::DIRECCION: return "direccion";
        case TokenType::INTENSITY: return "intensity";
        case TokenType::INTENSIDAD: return "intensidad";
        case TokenType::FALLOFF: return "falloff";
        case TokenType::CAIDA: return "caida";
        case TokenType::SDF: return "sdf";
        case TokenType::IDENT: return "identifier";
        case TokenType::INT: return "integer";
        case TokenType::FLOAT: return "float";
        case TokenType::STRING: return "string";
        case TokenType::END: return "end of file";
        default: return "token";
    }
}

Lexer::Lexer(const std::string& source)
    : source(source), pos(0), line(1), col(1), eof(false) {}

char Lexer::peek(int ahead) const {
    size_t idx = pos + ahead;
    return idx < source.size() ? source[idx] : '\0';
}

char Lexer::advance() {
    if (pos >= source.size()) {
        eof = true;
        return '\0';
    }
    char c = source[pos++];
    if (c == '\n') {
        line++;
        col = 1;
    } else {
        col++;
    }
    return c;
}

Token Lexer::make_token(TokenType type, const std::string& val) {
    return Token(type, val, line, col - (int)val.size());
}

void Lexer::skip_ws_and_comments() {
    while (pos < source.size()) {
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            advance();
        } else if (c == '/' && peek(1) == '/') {
            while (pos < source.size() && peek() != '\n') advance();
        } else if (c == '/' && peek(1) == '*') {
            advance(); advance(); // skip /*
            while (pos < source.size() && !(peek() == '*' && peek(1) == '/')) advance();
            if (pos < source.size()) { advance(); advance(); } // skip */
        } else {
            break;
        }
    }
}

Token Lexer::read_number() {
    std::string val;
    bool is_float = false;
    if (peek() == '-') val += advance();
    while (pos < source.size() && std::isdigit(peek())) {
        val += advance();
    }
    if (peek() == '.' && std::isdigit(peek(1))) {
        is_float = true;
        val += advance();
        while (pos < source.size() && std::isdigit(peek())) {
            val += advance();
        }
    }
    return make_token(is_float ? TokenType::FLOAT : TokenType::INT, val);
}

Token Lexer::read_string() {
    std::string val;
    advance();
    while (pos < source.size() && peek() != '"') {
        val += advance();
    }
    if (pos < source.size()) advance();
    return make_token(TokenType::STRING, val);
}

Token Lexer::read_ident() {
    std::string val;
    while (pos < source.size() && (std::isalnum(peek()) || peek() == '_')) {
        val += advance();
    }
    TokenType kw = lookupKeyword(val);
    return make_token(kw, val);
}

Token Lexer::next() {
    skip_ws_and_comments();
    if (pos >= source.size()) {
        return Token(TokenType::END, "", line, col);
    }

    char c = peek();
    int start_line = line;
    int start_col = col;

    if (std::isalpha(c) || c == '_') {
        return read_ident();
    }

    if (std::isdigit(c) || (c == '-' && std::isdigit(peek(1)))) {
        return read_number();
    }

    if (c == '"') {
        return read_string();
    }

    if (c == '.' && peek(1) == '.') {
        advance(); advance();
        return Token(TokenType::DOTDOT, "..", start_line, start_col);
    }

    advance();
    switch (c) {
        case '=': return Token(TokenType::ASSIGN, "=", start_line, start_col);
        case ':': return Token(TokenType::COLON, ":", start_line, start_col);
        case ',': return Token(TokenType::COMMA, ",", start_line, start_col);
        case ';': return Token(TokenType::SEMI, ";", start_line, start_col);
        case '.': return Token(TokenType::DOT, ".", start_line, start_col);
        case '+': return Token(TokenType::PLUS, "+", start_line, start_col);
        case '-': return Token(TokenType::MINUS, "-", start_line, start_col);
        case '*': return Token(TokenType::TIMES, "*", start_line, start_col);
        case '/': return Token(TokenType::DIVIDE, "/", start_line, start_col);
        case '%': return Token(TokenType::MOD, "%", start_line, start_col);
        case '^': return Token(TokenType::POW, "^", start_line, start_col);
        case '(': return Token(TokenType::LPAR, "(", start_line, start_col);
        case ')': return Token(TokenType::RPAR, ")", start_line, start_col);
        case '{': return Token(TokenType::LBRACE, "{", start_line, start_col);
        case '}': return Token(TokenType::RBRACE, "}", start_line, start_col);
        case '[': return Token(TokenType::LBRACK, "[", start_line, start_col);
        case ']': return Token(TokenType::RBRACK, "]", start_line, start_col);
        default:
            return Token(TokenType::ERROR, std::string(1, c), start_line, start_col);
    }
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (true) {
        Token t = next();
        tokens.push_back(t);
        if (t.type == TokenType::END || t.type == TokenType::ERROR) break;
    }
    return tokens;
}

} // namespace herm
