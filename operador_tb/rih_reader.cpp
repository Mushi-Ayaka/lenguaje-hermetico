#include "rih_reader.h"
#include "json_reader.h"
#include <fstream>
#include <sstream>
#include <unordered_map>

namespace herm {

static std::string readFile(const std::string& path) {
    std::ifstream f(path);
    if (!f) return {};
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

static Vec3 parseVec3(const JsonValue* arr) {
    Vec3 v = {};
    if (arr && arr->type == JsonValue::ARRAY) {
        if (arr->arrSize() > 0) v.x = (*arr)[0].asFloat();
        if (arr->arrSize() > 1) v.y = (*arr)[1].asFloat();
        if (arr->arrSize() > 2) v.z = (*arr)[2].asFloat();
    }
    return v;
}

static ColorRGB parseColor(const JsonValue* arr) {
    ColorRGB c = {};
    if (arr && arr->type == JsonValue::ARRAY) {
        if (arr->arrSize() > 0) c.r = (*arr)[0].asFloat();
        if (arr->arrSize() > 1) c.g = (*arr)[1].asFloat();
        if (arr->arrSize() > 2) c.b = (*arr)[2].asFloat();
    }
    return c;
}

static Expr parseExpr(const JsonValue* val) {
    Expr e;
    e.is_expr = false;
    e.constant = 0.0f;
    if (!val) return e;
    if (val->type == JsonValue::OBJECT) {
        auto* exprField = val->get("expr");
        if (exprField) {
            e.is_expr = true;
            e.expression = exprField->asString();
        }
    } else {
        e.constant = val->asFloat();
    }
    return e;
}

static BlendMode parseBlendMode(const std::string& s) {
    if (s == "add") return BlendMode::ADD;
    if (s == "replace") return BlendMode::REPLACE;
    return BlendMode::ALPHA;
}

static EmissionMode parseEmissionMode(const std::string& s) {
    if (s == "directional") return EmissionMode::DIRECTIONAL;
    return EmissionMode::ISOTROPIC;
}

static SdfType parseSdfType(const std::string& s) {
    static std::unordered_map<std::string, SdfType> map = {
        {"sphere", SdfType::SPHERE},
        {"box", SdfType::BOX},
        {"cylinder", SdfType::CYLINDER},
        {"capsule", SdfType::CAPSULE},
        {"torus", SdfType::TORUS},
        {"plane", SdfType::PLANE},
        {"cone", SdfType::CONE},
        {"rounded_box", SdfType::ROUNDED_BOX},
        {"octahedron", SdfType::OCTAHEDRON},
        {"tesseract", SdfType::TESSERACT},
        {"sphere4d", SdfType::SPHERE4D},
        {"union", SdfType::UNION},
        {"subtract", SdfType::SUBTRACT},
        {"intersect", SdfType::INTERSECT},
        {"smooth_union", SdfType::SMOOTH_UNION},
        {"smooth_subtract", SdfType::SMOOTH_SUBTRACT},
        {"smooth_intersect", SdfType::SMOOTH_INTERSECT},
        {"repeat", SdfType::REPEAT},
        {"twist", SdfType::TWIST},
        {"elongate", SdfType::ELONGATE},
        {"displace", SdfType::DISPLACE},
    };
    auto it = map.find(s);
    return (it != map.end()) ? it->second : SdfType::SPHERE;
}

static NodeType parseNodeType(const std::string& s) {
    if (s == "group") return NodeType::GROUP;
    if (s == "instance") return NodeType::INSTANCE;
    return NodeType::SDF;
}

static NodeMode parseNodeMode(const std::string& s) {
    if (s == "volume") return NodeMode::VOLUME;
    return NodeMode::SOLID;
}

static Expr parseTransformField(const JsonValue* arr, int idx) {
    if (!arr || arr->type != JsonValue::ARRAY || idx >= arr->arrSize())
        return {};
    return parseExpr(&(*arr)[idx]);
}

static SdfNode parseSdfNode(const JsonValue* jn) {
    SdfNode sn = {};
    auto* typeStr = jn->get("type");
    if (typeStr) sn.type = parseSdfType(typeStr->asString());
    auto* params = jn->get("params");
    if (params && params->type == JsonValue::ARRAY) {
        for (int i = 0; i < 4 && i < params->arrSize(); i++)
            sn.params[i] = parseExpr(&(*params)[i]);
    }
    auto* ca = jn->get("child_a");
    if (ca) sn.child_a = (u32)ca->asInt();
    auto* cb = jn->get("child_b");
    if (cb) sn.child_b = (u32)cb->asInt();
    auto* k = jn->get("k");
    if (k) sn.k = k->asFloat();
    auto* sp = jn->get("spacing");
    if (sp) sn.spacing = sp->asFloat();
    return sn;
}

static void parseTransform(const JsonValue* jt, Transform& t) {
    if (!jt) return;
    auto* tr = jt->get("translate");
    if (tr) {
        t.translate[0] = parseTransformField(tr, 0);
        t.translate[1] = parseTransformField(tr, 1);
        t.translate[2] = parseTransformField(tr, 2);
    }
    auto* rot = jt->get("rotate");
    if (rot) {
        t.rotate[0] = parseTransformField(rot, 0);
        t.rotate[1] = parseTransformField(rot, 1);
        t.rotate[2] = parseTransformField(rot, 2);
    }
    auto* scl = jt->get("scale");
    if (scl) {
        t.scale[0] = parseTransformField(scl, 0);
        t.scale[1] = parseTransformField(scl, 1);
        t.scale[2] = parseTransformField(scl, 2);
    }
}

static Light parseLight(const JsonValue* jl) {
    Light L;
    auto* id = jl->get("id");
    if (id) L.id = (u32)id->asInt();
    auto* name = jl->get("name");
    if (name) L.name = name->asString();
    auto* type = jl->get("type");
    if (type) {
        L.type = (type->asString() == "point") ? LightType::POINT : LightType::DIRECTIONAL;
    }
    L.direction = parseVec3(jl->get("direction"));
    L.position = parseVec3(jl->get("position"));
    L.color = parseColor(jl->get("color"));
    auto* intensity = jl->get("intensity");
    if (intensity) L.intensity = intensity->asFloat();
    auto* falloff = jl->get("falloff");
    if (falloff) L.falloff = falloff->asFloat();
    return L;
}

static Material parseMaterial(const JsonValue* jm) {
    Material m;
    auto* id = jm->get("id");
    if (id) m.id = (u32)id->asInt();
    auto* name = jm->get("name");
    if (name) m.name = name->asString();
    auto* tensor = jm->get("tensor");
    if (tensor && tensor->type == JsonValue::ARRAY) {
        for (int i = 0; i < 8 && i < tensor->arrSize(); i++) {
            m.tensor[i] = parseExpr(&(*tensor)[i]);
        }
    }

    auto* bl = jm->get("blend_mode");
    if (bl) m.blend_mode = parseBlendMode(bl->asString());
    return m;
}

static Node parseNode(const JsonValue* jn) {
    Node n;
    auto* id = jn->get("id");
    if (id) n.id = (u32)id->asInt();
    auto* name = jn->get("name");
    if (name) n.name = name->asString();
    auto* type = jn->get("type");
    if (type) n.type = parseNodeType(type->asString());
    auto* mode = jn->get("mode");
    if (mode) n.mode = parseNodeMode(mode->asString());
    parseTransform(jn->get("transform"), n.transform);
    auto* ds = jn->get("density_scale");
    if (ds) n.density_scale = ds->asFloat();
    auto* bm = jn->get("blend_mode");
    if (bm) n.blend_mode = parseBlendMode(bm->asString());
    auto* mat = jn->get("material");
    if (mat) n.material_id = (u32)mat->asInt();
    auto* def = jn->get("def");
    if (def) n.def_id = (u32)def->asInt();

    // Parse sdf — can be a single object or an array
    auto* sdf = jn->get("sdf");
    if (sdf) {
        if (sdf->type == JsonValue::ARRAY) {
            for (int i = 0; i < sdf->arrSize(); i++)
                n.sdf_nodes.push_back(parseSdfNode(&(*sdf)[i]));
        } else if (sdf->type == JsonValue::OBJECT) {
            n.sdf_nodes.push_back(parseSdfNode(sdf));
        }
    }

    // Parse children
    auto* children = jn->get("children");
    if (children && children->type == JsonValue::ARRAY) {
        for (int i = 0; i < children->arrSize(); i++)
            n.children.push_back((u32)(*children)[i].asInt());
    }

    return n;
}

Rih loadRih(const std::string& path) {
    Rih rih;

    std::string content = readFile(path);
    if (content.empty()) return rih;

    JsonReader reader(content);
    JsonValue root = reader.parse();
    if (root.type != JsonValue::OBJECT) return rih;

    auto* version = root.get("version");
    if (version) rih.version = (u32)version->asInt();

    auto* scene = root.get("scene");
    if (scene) {
        auto* axes = scene->get("axes");
        if (axes) {
            auto* x = axes->get("X"); if (x) rih.scene.width = (u32)x->asInt();
            auto* y = axes->get("Y"); if (y) rih.scene.height = (u32)y->asInt();
            auto* w = axes->get("W"); if (w) rih.scene.w_frames = (u32)w->asInt();
        }
        auto* time = scene->get("time");
        if (time) {
            auto* wmin = time->get("w_min"); if (wmin) rih.scene.w_min = wmin->asFloat();
            auto* wmax = time->get("w_max"); if (wmax) rih.scene.w_max = wmax->asFloat();
            auto* tu = time->get("t_unit"); if (tu) rih.scene.t_unit = tu->asString();
        }
        auto* cam = scene->get("camera");
        if (cam) {
            rih.scene.camera.position = parseVec3(cam->get("position"));
            rih.scene.camera.target = parseVec3(cam->get("target"));
            rih.scene.camera.up = parseVec3(cam->get("up"));
            auto* fov = cam->get("fov"); if (fov) rih.scene.camera.fov = fov->asFloat();
            auto* ap = cam->get("aperture"); if (ap) rih.scene.camera.aperture = ap->asFloat();
            auto* fd = cam->get("focal_dist"); if (fd) rih.scene.camera.focal_dist = fd->asFloat();
        }
        rih.scene.background = parseColor(scene->get("background"));
    }

    auto* lights = root.get("lights");
    if (lights && lights->type == JsonValue::ARRAY) {
        for (int i = 0; i < lights->arrSize(); i++)
            rih.lights.push_back(parseLight(&(*lights)[i]));
    }

    auto* materials = root.get("materials");
    if (materials && materials->type == JsonValue::ARRAY) {
        for (int i = 0; i < materials->arrSize(); i++)
            rih.materials.push_back(parseMaterial(&(*materials)[i]));
    }

    auto* defs = root.get("defs");
    if (defs && defs->type == JsonValue::ARRAY) {
        for (int i = 0; i < defs->arrSize(); i++)
            rih.defs.push_back(parseNode(&(*defs)[i]));
    }

    auto* nodes = root.get("nodes");
    if (nodes && nodes->type == JsonValue::ARRAY) {
        for (int i = 0; i < nodes->arrSize(); i++)
            rih.nodes.push_back(parseNode(&(*nodes)[i]));
    }

    return rih;
}

} // namespace herm
