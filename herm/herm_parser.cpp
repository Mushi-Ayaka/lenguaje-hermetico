#include "herm_parser.h"

namespace herm {

Parser::Parser(std::vector<Token> tokens, FileLoader loader)
    : tokens(std::move(tokens)), pos(0), loader(std::move(loader)) {}

Token Parser::peek() const {
    if (pos >= tokens.size()) return Token(TokenType::END, "", 0, 0);
    return tokens[pos];
}
Token Parser::advance() {
    if (pos >= tokens.size()) return Token(TokenType::END, "", 0, 0);
    return tokens[pos++];
}

static std::string locStr(const Token& t) {
    return "L" + std::to_string(t.line) + ":" + std::to_string(t.col);
}

Token Parser::expect(TokenType type) {
    if (peek().type != type) {
        auto t = peek();
        std::string msg = "se esperaba " + Lexer::typeName(type);
        if (t.type == TokenType::END) {
            msg += " pero se llego al final del archivo";
        } else {
            msg += " pero se encontro '" + t.value + "' en " + locStr(t);
        }
        throw ParseError(msg);
    }
    return advance();
}
bool Parser::skip(TokenType type) {
    if (peek().type == type) { advance(); return true; }
    return false;
}
std::string Parser::parse_string_or_ident() {
    if (peek().type == TokenType::STRING) return expect(TokenType::STRING).value;
    if (peek().type == TokenType::IDENT) return expect(TokenType::IDENT).value;
    // Accept keywords too (they were originally identifiers)
    auto t = peek();
    switch (t.type) {
        case TokenType::SCENE: case TokenType::AXES: case TokenType::CAMERA:
        case TokenType::BACKGROUND: case TokenType::MATERIAL: case TokenType::NODE:
        case TokenType::COLOR_BASE:
        case TokenType::LET: case TokenType::SEA:
        case TokenType::FN: case TokenType::FUNCION:
        case TokenType::DEF: case TokenType::INSTANCE: case TokenType::SDF:
        case TokenType::LIGHT: case TokenType::LUMBRE:
        case TokenType::TYPE: case TokenType::TIPO:
        case TokenType::POINT_LIGHT: case TokenType::PUNTUAL:
        case TokenType::DIRECTIONAL: case TokenType::DIRECCIONAL:
        case TokenType::DIRECTION: case TokenType::DIRECCION:
        case TokenType::INTENSITY: case TokenType::INTENSIDAD:
        case TokenType::FALLOFF: case TokenType::CAIDA:
        case TokenType::MODE: case TokenType::SOLID: case TokenType::VOLUME:
        case TokenType::VISIBLE: case TokenType::TRUE: case TokenType::FALSE:
        case TokenType::FOR: case TokenType::IMPORT: case TokenType::IMPORTAR:
        case TokenType::TENSOR:
        case TokenType::BLEND_MODE:
        case TokenType::ALPHA: case TokenType::ADD: case TokenType::REPLACE:
        case TokenType::POSITION: case TokenType::TARGET: case TokenType::UP:
        case TokenType::FOV: case TokenType::APERTURE: case TokenType::FOCAL_DIST:
        case TokenType::TRANSLATE: case TokenType::ROTATION: case TokenType::SCALE:
        case TokenType::DENSITY_SCALE:
        case TokenType::SPHERE: case TokenType::BOX: case TokenType::CYLINDER:
        case TokenType::CAPSULE: case TokenType::TORUS: case TokenType::PLANE:
        case TokenType::CONE: case TokenType::ROUNDED_BOX: case TokenType::OCTAHEDRON:
        case TokenType::TESSERACT: case TokenType::SPHERE4D:
        case TokenType::UNION: case TokenType::SUBTRACT: case TokenType::INTERSECT:
        case TokenType::SMOOTH_UNION: case TokenType::SMOOTH_SUBTRACT:
        case TokenType::SMOOTH_INTERSECT: case TokenType::REPEAT:
        case TokenType::TWIST: case TokenType::ELONGATE: case TokenType::DISPLACE:
            advance(); return t.value;
        default:
            throw ParseError("se esperaba string/identificador pero se encontro '" + t.value + "' en " + locStr(t));
    }
}
Color3 Parser::parse_color3() {
    expect(TokenType::LBRACK);
    auto r = parse_expr(); expect(TokenType::COMMA);
    auto g = parse_expr(); expect(TokenType::COMMA);
    auto b = parse_expr(); expect(TokenType::RBRACK);
    return {r, g, b};
}
Vec3Expr Parser::parse_vec3() {
    expect(TokenType::LBRACK);
    auto x = parse_expr(); expect(TokenType::COMMA);
    auto y = parse_expr(); expect(TokenType::COMMA);
    auto z = parse_expr(); expect(TokenType::RBRACK);
    return {x, y, z};
}

ExprNode Parser::parse_expr() {
    if (peek().type == TokenType::END) {
        throw ParseError("se esperaba expresion pero se llego al final del archivo");
    }
    int start_line = peek().line;
    std::string t;
    int depth = 0;
    while (peek().type != TokenType::END) {
        auto tok = peek();
        if (tok.type == TokenType::LPAR || tok.type == TokenType::LBRACK) depth++;
        if (tok.type == TokenType::RPAR || tok.type == TokenType::RBRACK) depth--;
        if (depth < 0) break;
        if (depth == 0 && (tok.type == TokenType::COMMA || tok.type == TokenType::SEMI || tok.type == TokenType::COLON)) break;
        if (tok.type == TokenType::RBRACE) break;
        if (!t.empty()) t += " ";
        t += tok.value;
        advance();
    }
    while (!t.empty() && t.back() == ' ') t.pop_back();
    return {t, start_line};
}

// ============================================================================
// TOP-LEVEL
// ============================================================================

AstProgram Parser::parse() {
    AstProgram p;
    while (peek().type != TokenType::END) {
        switch (peek().type) {
            case TokenType::SCENE:     p.scene = parse_scene(); break;
            case TokenType::LET:
            case TokenType::SEA:       p.lets.push_back(parse_let()); break;
            case TokenType::FN:
            case TokenType::FUNCION:   p.fns.push_back(parse_fn()); break;
            case TokenType::LIGHT:
            case TokenType::LUMBRE:    p.lights.push_back(parse_light()); break;
            case TokenType::MATERIAL:  p.materials.push_back(parse_material()); break;
            case TokenType::DEF:       p.defs.push_back(parse_def()); break;
            case TokenType::NODE:      p.nodes.push_back(parse_node()); break;
            case TokenType::INSTANCE:  p.nodes.push_back(parse_instance()); break;
            case TokenType::IMPORT:
            case TokenType::IMPORTAR:  parse_import(p); break;
            default: { auto t = peek(); throw ParseError("elemento inesperado '" + t.value + "' en " + locStr(t)); }
        }
    }
    return p;
}

void Parser::parse_import(AstProgram& prog) {
    advance(); // consume import/importar
    auto path = expect(TokenType::STRING).value;
    skip(TokenType::SEMI);

    if (!loader) {
        throw ParseError("import '" + path + "' no soportado (sin cargador de archivos)");
    }

    std::string source;
    try {
        source = loader(path);
    } catch (const std::exception& e) {
        throw ParseError("import '" + path + "' error: " + e.what());
    }

    if (source.empty()) {
        throw ParseError("import '" + path + "' no encontrado");
    }

    Lexer sub_lexer(source);
    auto sub_tokens = sub_lexer.tokenize();
    Parser sub_parser(std::move(sub_tokens), loader);
    auto sub_prog = sub_parser.parse();

    // Merge imported materials, defs, and nodes
    for (auto& m : sub_prog.materials) {
        prog.materials.push_back(std::move(m));
    }
    for (auto& d : sub_prog.defs) {
        prog.defs.push_back(std::move(d));
    }
    // Merge fns (pure math functions)
    for (auto& f : sub_prog.fns) {
        prog.fns.push_back(std::move(f));
    }
    for (auto& lt : sub_prog.lets) {
        prog.lets.push_back(std::move(lt));
    }
    for (auto& n : sub_prog.nodes) {
        prog.nodes.push_back(std::move(n));
    }
    // Scene is not merged (first scene wins)
    if (!prog.scene.has_value() && sub_prog.scene.has_value()) {
        prog.scene = std::move(sub_prog.scene);
    }
}

std::unique_ptr<AstNode> Parser::parse_instance() {
    expect(TokenType::INSTANCE);
    auto n = std::make_unique<AstNode>();
    n->def_ref = parse_string_or_ident();

    // Parse optional arguments: (arg1, arg2, ...)
    if (peek().type == TokenType::LPAR) {
        expect(TokenType::LPAR);
        while (peek().type != TokenType::RPAR && peek().type != TokenType::END) {
            if (!n->args.empty()) expect(TokenType::COMMA);
            n->args.push_back(parse_expr());
        }
        expect(TokenType::RPAR);
    }

    if (peek().type == TokenType::LBRACE) {
        expect(TokenType::LBRACE);
        while (peek().type != TokenType::RBRACE && peek().type != TokenType::END) {
            auto t = peek().type;
            if (t == TokenType::POSITION || t == TokenType::TRANSLATE) {
                advance(); expect(TokenType::COLON);
                if (!n->instance_transform) n->instance_transform = AstTransform{};
                n->instance_transform->translate = parse_vec3(); skip(TokenType::SEMI);
            } else if (t == TokenType::ROTATION) {
                advance(); expect(TokenType::COLON);
                if (!n->instance_transform) n->instance_transform = AstTransform{};
                n->instance_transform->rotate = parse_vec3(); skip(TokenType::SEMI);
            } else if (t == TokenType::SCALE) {
                advance(); expect(TokenType::COLON);
                if (!n->instance_transform) n->instance_transform = AstTransform{};
                n->instance_transform->scale = parse_vec3(); skip(TokenType::SEMI);
            } else if (t == TokenType::MATERIAL) {
                advance(); expect(TokenType::COLON); n->material = parse_string_or_ident(); skip(TokenType::SEMI);
            } else break;
        }
        expect(TokenType::RBRACE);
    }
    skip(TokenType::SEMI);
    return n;
}

AstScene Parser::parse_scene() {
    expect(TokenType::SCENE);
    if (peek().type == TokenType::STRING) advance();
    expect(TokenType::LBRACE);
    AstScene s;
    while (peek().type != TokenType::RBRACE && peek().type != TokenType::END) {
        switch (peek().type) {
            case TokenType::AXES: {
                expect(TokenType::AXES); expect(TokenType::LBRACE);
                while (peek().type != TokenType::RBRACE && peek().type != TokenType::END) {
                    auto a = advance().value;
                    expect(TokenType::COLON); auto v = parse_expr();
                    if (a == "X" || a == "x") s.width = v;
                    else if (a == "Y" || a == "y") s.height = v;
                    else if (a == "W" || a == "w") s.w_frames = v;
                    skip(TokenType::COMMA);
                }
                expect(TokenType::RBRACE);
                break;
            }
            case TokenType::CAMERA: s.camera = parse_camera(); break;
            case TokenType::BACKGROUND:
                expect(TokenType::BACKGROUND); expect(TokenType::COLON);
                s.background = parse_color3(); skip(TokenType::SEMI);
                break;
            default: { auto t = peek(); throw ParseError("inesperado en escena: '" + t.value + "' en " + locStr(t)); }
        }
    }
    expect(TokenType::RBRACE);
    return s;
}

AstCamera Parser::parse_camera() {
    expect(TokenType::CAMERA); expect(TokenType::LBRACE);
    AstCamera c;
    while (peek().type != TokenType::RBRACE && peek().type != TokenType::END) {
        auto f = [&](auto& opt, auto getter) {
            advance(); expect(TokenType::COLON); opt = getter(); skip(TokenType::SEMI);
        };
        switch (peek().type) {
            case TokenType::POSITION: f(c.position, [&]{ return parse_vec3(); }); break;
            case TokenType::TARGET:   f(c.target, [&]{ return parse_vec3(); }); break;
            case TokenType::UP:       f(c.up, [&]{ return parse_vec3(); }); break;
            case TokenType::FOV:      f(c.fov, [&]{ return parse_expr(); }); break;
            case TokenType::APERTURE: f(c.aperture, [&]{ return parse_expr(); }); break;
            case TokenType::FOCAL_DIST: f(c.focal_dist, [&]{ return parse_expr(); }); break;
            default: { auto t = peek(); throw ParseError("inesperado en camara: '" + t.value + "' en " + locStr(t)); }
        }
    }
    expect(TokenType::RBRACE);
    return c;
}

AstLet Parser::parse_let() {
    advance(); // consume let / sea
    auto name = expect(TokenType::IDENT).value;
    expect(TokenType::ASSIGN);
    auto val = parse_expr();
    skip(TokenType::SEMI);
    AstLet L;
    L.name = name;
    L.value = val;
    return L;
}

AstFn Parser::parse_fn() {
    advance(); // consume fn / funcion
    AstFn fn;
    fn.name = expect(TokenType::STRING).value;

    // Parameters: (p1, p2, ...)
    expect(TokenType::LPAR);
    while (peek().type != TokenType::RPAR && peek().type != TokenType::END) {
        if (!fn.params.empty()) skip(TokenType::COMMA);
        fn.params.push_back(expect(TokenType::IDENT).value);
    }
    expect(TokenType::RPAR);

    // Body: internal lets (DL-14) + final expression, closed by }
    expect(TokenType::LBRACE);
    while (peek().type == TokenType::LET) {
        AstLet L = parse_let();
        fn.body_lets.push_back({L.name, L.value.text});
    }
    if (peek().type == TokenType::RBRACE || peek().type == TokenType::END) {
        throw ParseError("fn '" + fn.name + "' debe terminar con una expresion de retorno en " + locStr(peek()));
    }
    fn.body_expr = parse_expr().text;
    skip(TokenType::SEMI);
    expect(TokenType::RBRACE);

    return fn;
}

AstLight Parser::parse_light() {
    advance(); // consume light / lumbre
    auto n = expect(TokenType::STRING).value;
    expect(TokenType::LBRACE);
    AstLight L; L.name = n;
    while (peek().type != TokenType::RBRACE && peek().type != TokenType::END) {
        auto fv = [&]() { return parse_expr(); };
        auto fc = [&]() { return parse_color3(); };
        auto fs = [&]() { return parse_string_or_ident(); };
        auto fv3 = [&]() { return parse_vec3(); };
        switch (peek().type) {
            case TokenType::TYPE:
            case TokenType::TIPO:
                advance();
                expect(TokenType::COLON);
                L.type = fs();
                skip(TokenType::SEMI);
                break;
            case TokenType::DIRECTION: advance(); expect(TokenType::COLON); L.direction = fv3(); skip(TokenType::SEMI); break;
            case TokenType::POSITION:  advance(); expect(TokenType::COLON); L.position  = fv3(); skip(TokenType::SEMI); break;
            case TokenType::BASE_COLOR:
            case TokenType::COLOR_BASE: advance(); expect(TokenType::COLON); L.color     = fc();  skip(TokenType::SEMI); break;
            case TokenType::INTENSITY: advance(); expect(TokenType::COLON); L.intensity = fv();  skip(TokenType::SEMI); break;
            case TokenType::FALLOFF:   advance(); expect(TokenType::COLON); L.falloff   = fv();  skip(TokenType::SEMI); break;
            default: { auto t = peek(); throw ParseError("inesperado en light: '" + t.value + "' en " + locStr(t)); }
        }
    }
    expect(TokenType::RBRACE);
    return L;
}

AstMaterial Parser::parse_material() {
    expect(TokenType::MATERIAL);
    auto n = expect(TokenType::STRING).value;
    expect(TokenType::LBRACE);
    AstMaterial m; m.name = n;
    while (peek().type != TokenType::RBRACE && peek().type != TokenType::END) {
        auto fv = [&]() { return parse_expr(); };
        auto fc = [&]() { return parse_color3(); };
        auto fs = [&]() { return parse_string_or_ident(); };
        auto fv3 = [&]() { return parse_vec3(); };
        switch (peek().type) {
            case TokenType::TENSOR: {
                advance(); expect(TokenType::COLON);
                expect(TokenType::LBRACK);
                std::vector<ExprNode> tensor_exprs;
                while (peek().type != TokenType::RBRACK && peek().type != TokenType::END) {
                    if (!tensor_exprs.empty()) expect(TokenType::COMMA);
                    tensor_exprs.push_back(parse_expr());
                }
                expect(TokenType::RBRACK);
                m.tensor = tensor_exprs;
                skip(TokenType::SEMI);
                break;
            }
            case TokenType::BLEND_MODE:  advance(); expect(TokenType::COLON); m.blend_mode = fs(); skip(TokenType::SEMI); break;
            case TokenType::BASE_COLOR:
            case TokenType::COLOR_BASE: advance(); expect(TokenType::COLON); m.base_color = fc(); skip(TokenType::SEMI); break;
            case TokenType::ROUGHNESS:
            case TokenType::RUGOSIDAD:  advance(); expect(TokenType::COLON); m.roughness = fv(); skip(TokenType::SEMI); break;
            case TokenType::METALLIC:
            case TokenType::METALICO:   advance(); expect(TokenType::COLON); m.metallic  = fv(); skip(TokenType::SEMI); break;
            case TokenType::EMISSION:
            case TokenType::EMISION:    advance(); expect(TokenType::COLON); m.emission  = fc(); skip(TokenType::SEMI); break;
            case TokenType::OPACITY:
            case TokenType::OPACIDAD:   advance(); expect(TokenType::COLON); m.opacity   = fv(); skip(TokenType::SEMI); break;
            default: { auto t = peek(); throw ParseError("inesperado en material: '" + t.value + "' en " + locStr(t)); }
        }
    }
    expect(TokenType::RBRACE);
    return m;
}

AstDef Parser::parse_def() {
    expect(TokenType::DEF);
    auto n = expect(TokenType::STRING).value;
    AstDef d; d.name = n;

    // Parse optional parameters: (param1, param2, ...)
    if (peek().type == TokenType::LPAR) {
        expect(TokenType::LPAR);
        while (peek().type != TokenType::RPAR && peek().type != TokenType::END) {
            if (!d.params.empty()) expect(TokenType::COMMA);
            d.params.push_back(expect(TokenType::IDENT).value);
        }
        expect(TokenType::RPAR);
    }

    expect(TokenType::LBRACE);
    d.node = std::make_unique<AstNode>();
    d.node->name = n;
    parse_node_body(*d.node);
    expect(TokenType::RBRACE);
    return d;
}

std::unique_ptr<AstNode> Parser::parse_node() {
    expect(TokenType::NODE);
    auto n = std::make_unique<AstNode>();
    if (peek().type == TokenType::STRING || peek().type == TokenType::IDENT) n->name = advance().value;
    expect(TokenType::LBRACE);
    parse_node_body(*n);
    expect(TokenType::RBRACE);
    return n;
}

void Parser::parse_node_body(AstNode& node) {
    while (peek().type != TokenType::RBRACE && peek().type != TokenType::END) {
        switch (peek().type) {
            case TokenType::MODE:
                advance(); expect(TokenType::COLON); node.mode = advance().value; skip(TokenType::SEMI); break;
            case TokenType::SDF:
                advance(); expect(TokenType::COLON); node.sdf = parse_sdf(); skip(TokenType::SEMI); break;
            case TokenType::MATERIAL:
                advance(); expect(TokenType::COLON); node.material = parse_string_or_ident(); skip(TokenType::SEMI); break;
            case TokenType::DENSITY_SCALE:
                advance(); expect(TokenType::COLON); node.density_scale = parse_expr(); skip(TokenType::SEMI); break;
            case TokenType::BLEND_MODE:
                advance(); expect(TokenType::COLON); node.blend_mode = parse_string_or_ident(); skip(TokenType::SEMI); break;
            case TokenType::POSITION:
            case TokenType::TRANSLATE:
                advance(); expect(TokenType::COLON);
                if (!node.transform) node.transform = AstTransform{};
                node.transform->translate = parse_vec3(); skip(TokenType::SEMI); break;
            case TokenType::ROTATION:
                advance(); expect(TokenType::COLON);
                if (!node.transform) node.transform = AstTransform{};
                node.transform->rotate = parse_vec3(); skip(TokenType::SEMI); break;
            case TokenType::SCALE:
                advance(); expect(TokenType::COLON);
                if (!node.transform) node.transform = AstTransform{};
                node.transform->scale = parse_vec3(); skip(TokenType::SEMI); break;
            case TokenType::INSTANCE: {
                advance();
                auto child = std::make_unique<AstNode>();
                child->def_ref = parse_string_or_ident();
                if (peek().type == TokenType::LPAR) {
                    expect(TokenType::LPAR);
                    while (peek().type != TokenType::RPAR && peek().type != TokenType::END) {
                        if (!child->args.empty()) expect(TokenType::COMMA);
                        child->args.push_back(parse_expr());
                    }
                    expect(TokenType::RPAR);
                }
                if (peek().type == TokenType::LBRACE) {
                    expect(TokenType::LBRACE);
                    while (peek().type != TokenType::RBRACE && peek().type != TokenType::END) {
                        auto t = peek().type;
                        if (t == TokenType::POSITION || t == TokenType::TRANSLATE) {
                            advance(); expect(TokenType::COLON);
                            if (!child->instance_transform) child->instance_transform = AstTransform{};
                            child->instance_transform->translate = parse_vec3(); skip(TokenType::SEMI);
                        } else if (t == TokenType::ROTATION) {
                            advance(); expect(TokenType::COLON);
                            if (!child->instance_transform) child->instance_transform = AstTransform{};
                            child->instance_transform->rotate = parse_vec3(); skip(TokenType::SEMI);
                        } else if (t == TokenType::SCALE) {
                            advance(); expect(TokenType::COLON);
                            if (!child->instance_transform) child->instance_transform = AstTransform{};
                            child->instance_transform->scale = parse_vec3(); skip(TokenType::SEMI);
                        } else break;
                    }
                    expect(TokenType::RBRACE);
                }
                skip(TokenType::SEMI);
                node.children.push_back(std::move(child));
                break;
            }
            case TokenType::NODE:
                node.children.push_back(parse_node());
                break;
            default: {
                auto t = peek();
                throw ParseError("inesperado en nodo: '" + t.value + "' en " + locStr(t));
            }
        }
    }
}

// ============================================================================
// SDF
// ============================================================================

static SdfOpType tk2op(TokenType t) {
    switch (t) {
        case TokenType::UNION:   return SdfOpType::UNION;
        case TokenType::SUBTRACT: return SdfOpType::SUBTRACT;
        case TokenType::INTERSECT: return SdfOpType::INTERSECT;
        case TokenType::SMOOTH_UNION: return SdfOpType::SMOOTH_UNION;
        case TokenType::SMOOTH_SUBTRACT: return SdfOpType::SMOOTH_SUBTRACT;
        case TokenType::SMOOTH_INTERSECT: return SdfOpType::SMOOTH_INTERSECT;
        case TokenType::REPEAT:   return SdfOpType::REPEAT;
        case TokenType::TWIST:    return SdfOpType::TWIST;
        case TokenType::ELONGATE: return SdfOpType::ELONGATE;
        case TokenType::DISPLACE: return SdfOpType::DISPLACE;
        default: throw ParseError("no es un operador SDF");
    }
}
static SdfPrimType tk2prim(TokenType t) {
    switch (t) {
        case TokenType::SPHERE:  return SdfPrimType::SPHERE;
        case TokenType::BOX:     return SdfPrimType::BOX;
        case TokenType::CYLINDER: return SdfPrimType::CYLINDER;
        case TokenType::CAPSULE: return SdfPrimType::CAPSULE;
        case TokenType::TORUS:   return SdfPrimType::TORUS;
        case TokenType::PLANE:   return SdfPrimType::PLANE;
        case TokenType::CONE:    return SdfPrimType::CONE;
        case TokenType::ROUNDED_BOX: return SdfPrimType::ROUNDED_BOX;
        case TokenType::OCTAHEDRON: return SdfPrimType::OCTAHEDRON;
        case TokenType::TESSERACT: return SdfPrimType::TESSERACT;
        case TokenType::SPHERE4D: return SdfPrimType::SPHERE4D;
        case TokenType::CUSTOM:   return SdfPrimType::CUSTOM;
        default: throw ParseError("no es una primitiva SDF");
    }
}
static bool isOp(TokenType t) {
    switch (t) {
        case TokenType::UNION: case TokenType::SUBTRACT: case TokenType::INTERSECT:
        case TokenType::SMOOTH_UNION: case TokenType::SMOOTH_SUBTRACT: case TokenType::SMOOTH_INTERSECT:
        case TokenType::REPEAT: case TokenType::TWIST: case TokenType::ELONGATE: case TokenType::DISPLACE:
            return true;
        default: return false;
    }
}
static bool isPrim(TokenType t) {
    switch (t) {
        case TokenType::SPHERE: case TokenType::BOX: case TokenType::CYLINDER:
        case TokenType::CAPSULE: case TokenType::TORUS: case TokenType::PLANE:
        case TokenType::CONE: case TokenType::ROUNDED_BOX: case TokenType::OCTAHEDRON:
        case TokenType::TESSERACT:         case TokenType::SPHERE4D:
        case TokenType::CUSTOM:
            return true;
        default: return false;
    }
}

std::unique_ptr<AstSdf> Parser::parse_sdf() {
    auto sdf = std::make_unique<AstSdf>();
    auto t = peek().type;
    if (isOp(t)) {
        SdfOperator op;
        op.type = tk2op(t); advance();
        expect(TokenType::LPAR);
        op.left = parse_sdf();
        if (op.type == SdfOpType::REPEAT || op.type == SdfOpType::TWIST ||
            op.type == SdfOpType::ELONGATE || op.type == SdfOpType::DISPLACE) {
            expect(TokenType::COMMA); op.param = parse_expr();
        } else {
            expect(TokenType::COMMA); op.right = parse_sdf();
            if (op.type == SdfOpType::SMOOTH_UNION || op.type == SdfOpType::SMOOTH_SUBTRACT ||
                op.type == SdfOpType::SMOOTH_INTERSECT) {
                if (peek().type == TokenType::COMMA) { advance(); op.k = parse_expr(); }
            }
        }
        expect(TokenType::RPAR);
        sdf->data = std::move(op);
    } else if (isPrim(t)) {
        SdfPrimitive prim;
        prim.type = tk2prim(t); advance();
        expect(TokenType::LPAR);
        while (peek().type != TokenType::RPAR && peek().type != TokenType::END) {
            if (!prim.params.empty()) expect(TokenType::COMMA);
            prim.params.push_back(parse_expr());
        }
        expect(TokenType::RPAR);
        sdf->data = std::move(prim);
    } else {
        auto tok = peek();
        std::string ctx = "se esperaba primitiva u operador SDF pero se encontro '" + tok.value + "'";
        if (tok.line > 0) ctx += " en " + locStr(tok);
        throw ParseError(ctx);
    }
    return sdf;
}

} // namespace herm
