#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace herm {

enum class TokenType {
    // Keywords (bilingual)
    SCENE, ESCENA,
    AXES, EJES,
    CAMERA, CAMARA,
    BACKGROUND, FONDO,
    MATERIAL,
    NODE, NODO,
    DEF,
    INSTANCE, INSTANCIA,
    SDF,
    MODE, MODO,
    SOLID, SOLIDO,
    VOLUME, VOLUMEN,
    VISIBLE,
    TRUE, FALSE,
    FOR, PARA,
    IMPORT, IMPORTAR,
    LET, SEA,
    FN, FUNCION,
    LIGHT, LUMBRE,
    TYPE, TIPO,
    POINT_LIGHT, PUNTUAL,
    DIRECTION, DIRECCION,
    INTENSITY, INTENSIDAD,
    FALLOFF, CAIDA,

    // Material fields
    TENSOR,
    BASE_COLOR, COLOR_BASE,
    ROUGHNESS, RUGOSIDAD,
    METALLIC, METALICO,
    EMISSION, EMISION,
    EMISSION_MODE, MODO_EMISION,
    EMISSION_DIR, DIR_EMISION,
    ISOTROPIC, ISOTROPICO,
    DIRECTIONAL, DIRECCIONAL,
    IOR,
    TRANSMISSION, TRANSMISION,
    OPACITY, OPACIDAD,
    SUBSURFACE, SUBSUPERFICIE,
    BLEND_MODE, MODO_MEZCLA,
    ALPHA,
    ADD, SUMAR,
    REPLACE, REEMPLAZAR,

    // Transform fields
    POSITION, POSICION,
    TARGET, OBJETIVO,
    UP, ARRIBA,
    FOV,
    APERTURE, APERTURA,
    FOCAL_DIST, DISTANCIA_FOCAL,
    TRANSLATE, TRASLACION,
    ROTATION, ROTACION,
    SCALE, ESCALA,
    DENSITY_SCALE, ESCALA_DENSIDAD,

    // SDF primitives
    SPHERE, ESFERA,
    BOX, CAJA,
    CYLINDER, CILINDRO,
    CAPSULE, CAPSULA,
    TORUS, TORO,
    PLANE, PLANO,
    CONE, CONO,
    ROUNDED_BOX, CAJA_REDONDEADA,
    OCTAHEDRON, OCTAEDRO,
    TESSERACT, HIPERCUBO,
    SPHERE4D, ESFERA4D,

    // SDF operators
    UNION,
    SUBTRACT, RESTA,
    INTERSECT, INTERSECCION,
    SMOOTH_UNION, UNION_SUAVE,
    SMOOTH_SUBTRACT, RESTA_SUAVE,
    SMOOTH_INTERSECT, INTERSECCION_SUAVE,
    REPEAT, REPETIR,
    TWIST, TORSION,
    ELONGATE, ALARGAR,
    DISPLACE, DESPLAZAR,

    // Literals
    IDENT, INT, FLOAT, STRING,

    // Operators
    ASSIGN,     // =
    COLON,      // :
    COMMA,      // ,
    SEMI,       // ;
    DOT,        // .
    DOTDOT,     // ..
    PLUS, MINUS, TIMES, DIVIDE, MOD, POW,

    // Delimiters
    LPAR, RPAR,
    LBRACE, RBRACE,
    LBRACK, RBRACK,

    // Special
    END, ERROR
};

struct Token {
    TokenType type;
    std::string value;
    int line;
    int col;

    Token(TokenType t, const std::string& v, int l = 0, int c = 0)
        : type(t), value(v), line(l), col(c) {}
};

class Lexer {
public:
    Lexer(const std::string& source);

    Token next();
    std::vector<Token> tokenize();

    static std::string typeName(TokenType t);

private:
    char peek(int ahead = 0) const;
    char advance();
    void skip_ws_and_comments();
    Token make_token(TokenType type, const std::string& val);
    Token read_number();
    Token read_string();
    Token read_ident();

    std::string source;
    size_t pos;
    int line;
    int col;
    bool eof;

    static const std::unordered_map<std::string, TokenType> keywords_es;
    static const std::unordered_map<std::string, TokenType> keywords_en;
    static TokenType lookupKeyword(const std::string& val);
};

} // namespace herm
