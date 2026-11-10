#include "herm_rih.h"
#include "herm_eval.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstring>

namespace herm {

// ============================================================================
// JSON Writer
// ============================================================================

static std::string jsonEscape(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        if (c == '"') out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else if (c == '\n') out += "\\n";
        else out += c;
    }
    return out;
}

static void jsonColor(std::ostringstream& os, const ColorRGB& c) {
    os << "[" << std::fixed << std::setprecision(4) << c.r << ", " << c.g << ", " << c.b << "]";
}

static void jsonVec3(std::ostringstream& os, const Vec3& v) {
    os << "[" << std::fixed << std::setprecision(4) << v.x << ", " << v.y << ", " << v.z << "]";
}

static void jsonExpr(std::ostringstream& os, const Expr& e, const std::string& indent) {
    if (e.is_expr) {
        os << "{ \"expr\": \"" << jsonEscape(e.expression) << "\" }";
    } else {
        os << std::fixed << std::setprecision(4) << e.constant;
    }
}

static void jsonExpr3(std::ostringstream& os, const Expr e[3], const std::string& indent) {
    os << "[";
    jsonExpr(os, e[0], indent); os << ", ";
    jsonExpr(os, e[1], indent); os << ", ";
    jsonExpr(os, e[2], indent);
    os << "]";
}

static void jsonExpr8(std::ostringstream& os, const Expr e[8], const std::string& indent) {
    os << "[\n";
    for (int i = 0; i < 8; i++) {
        os << indent << "  ";
        jsonExpr(os, e[i], indent + "  ");
        if (i < 7) os << ",\n"; else os << "\n";
    }
    os << indent << "]";
}

static void jsonTransform(std::ostringstream& os, const Transform& t, const std::string& indent) {
    os << "{\n";
    os << indent << "  \"translate\": "; jsonExpr3(os, t.translate, indent + "  "); os << ",\n";
    os << indent << "  \"rotate\": "; jsonExpr3(os, t.rotate, indent + "  "); os << ",\n";
    os << indent << "  \"scale\": "; jsonExpr3(os, t.scale, indent + "  ");
    os << "\n" << indent << "}";
}

static void jsonSdf(std::ostringstream& os, const SdfNode& sdf, const std::string& indent) {
    os << "{\n";

    auto typeStr = [](SdfType t) -> std::string {
        switch (t) {
            case SdfType::SPHERE:  return "sphere";
            case SdfType::BOX:     return "box";
            case SdfType::CYLINDER: return "cylinder";
            case SdfType::CAPSULE: return "capsule";
            case SdfType::TORUS:   return "torus";
            case SdfType::PLANE:   return "plane";
            case SdfType::CONE:    return "cone";
            case SdfType::ROUNDED_BOX: return "rounded_box";
            case SdfType::OCTAHEDRON: return "octahedron";
            case SdfType::TESSERACT: return "tesseract";
            case SdfType::SPHERE4D: return "sphere4d";
            case SdfType::CUSTOM:  return "custom";
            case SdfType::UNION:   return "union";
            case SdfType::SUBTRACT: return "subtract";
            case SdfType::INTERSECT: return "intersect";
            case SdfType::SMOOTH_UNION: return "smooth_union";
            case SdfType::SMOOTH_SUBTRACT: return "smooth_subtract";
            case SdfType::SMOOTH_INTERSECT: return "smooth_intersect";
            case SdfType::REPEAT:  return "repeat";
            case SdfType::TWIST:   return "twist";
            case SdfType::ELONGATE: return "elongate";
            case SdfType::DISPLACE: return "displace";
        }
        return "unknown";
    };

    os << indent << "  \"type\": \"" << typeStr(sdf.type) << "\"";
    os << ",\n" << indent << "  \"params\": [";
    jsonExpr(os, sdf.params[0], indent + "  "); os << ", ";
    jsonExpr(os, sdf.params[1], indent + "  "); os << ", ";
    jsonExpr(os, sdf.params[2], indent + "  "); os << ", ";
    jsonExpr(os, sdf.params[3], indent + "  ");
    os << "]";

    if (sdf.child_a) os << ",\n" << indent << "  \"child_a\": " << *sdf.child_a;
    if (sdf.child_b) os << ",\n" << indent << "  \"child_b\": " << *sdf.child_b;
    if (sdf.k) os << ",\n" << indent << "  \"k\": " << std::fixed << std::setprecision(4) << *sdf.k;
    if (sdf.spacing) os << ",\n" << indent << "  \"spacing\": " << std::fixed << std::setprecision(4) << *sdf.spacing;
    if (sdf.displace_func) os << ",\n" << indent << "  \"displace\": \"" << jsonEscape(*sdf.displace_func) << "\"";

    os << "\n" << indent << "}";
}

std::string rihToJsonString(const Rih& rih) {
    std::ostringstream os;
    os << std::setprecision(6);

    os << "{\n";
    os << "  \"version\": " << (int)rih.version << ",\n";

    // Scene
    os << "  \"scene\": {\n";
    os << "    \"axes\": { \"X\": " << rih.scene.width << ", \"Y\": " << rih.scene.height << ", \"W\": " << rih.scene.w_frames << " },\n";
    os << "    \"time\": { \"w_min\": " << rih.scene.w_min << ", \"w_max\": " << rih.scene.w_max << ", \"t_unit\": \"" << rih.scene.t_unit << "\" },\n";
    os << "    \"camera\": {\n";
    os << "      \"position\": "; jsonVec3(os, rih.scene.camera.position); os << ",\n";
    os << "      \"target\": "; jsonVec3(os, rih.scene.camera.target); os << ",\n";
    os << "      \"up\": "; jsonVec3(os, rih.scene.camera.up); os << ",\n";
    os << "      \"fov\": " << rih.scene.camera.fov << ",\n";
    os << "      \"aperture\": " << rih.scene.camera.aperture << ",\n";
    os << "      \"focal_dist\": " << rih.scene.camera.focal_dist << "\n";
    os << "    },\n";
    os << "    \"background\": "; jsonColor(os, rih.scene.background); os << "\n";
    os << "  },\n";

    // Lights
    os << "  \"lights\": [\n";
    for (size_t i = 0; i < rih.lights.size(); i++) {
        auto& L = rih.lights[i];
        os << "    {\n";
        os << "      \"id\": " << L.id << ",\n";
        os << "      \"name\": \"" << jsonEscape(L.name) << "\",\n";
        os << "      \"type\": \"" << (L.type == LightType::DIRECTIONAL ? "directional" : "point") << "\",\n";
        os << "      \"direction\": "; jsonVec3(os, L.direction); os << ",\n";
        os << "      \"position\": "; jsonVec3(os, L.position); os << ",\n";
        os << "      \"color\": "; jsonColor(os, L.color); os << ",\n";
        os << "      \"intensity\": " << L.intensity << ",\n";
        os << "      \"falloff\": " << L.falloff << "\n";
        os << "    }" << (i < rih.lights.size() - 1 ? "," : "") << "\n";
    }
    os << "  ],\n";

    // Materials
    os << "  \"materials\": [\n";
    for (size_t i = 0; i < rih.materials.size(); i++) {
        auto& m = rih.materials[i];
        os << "    {\n";
        os << "      \"id\": " << m.id << ",\n";
        os << "      \"name\": \"" << jsonEscape(m.name) << "\",\n";
        os << "      \"tensor\": "; jsonExpr8(os, m.tensor, "      "); os << ",\n";
        os << "      \"blend_mode\": \"" << (m.blend_mode == BlendMode::ADD ? "add" : m.blend_mode == BlendMode::REPLACE ? "replace" : "alpha") << "\"\n";
        os << "    }" << (i < rih.materials.size() - 1 ? "," : "") << "\n";
    }
    os << "  ],\n";

    auto writeNodeList = [&](const std::vector<Node>& nodes, const std::string& name) {
        os << "  \"" << name << "\": [\n";
        for (size_t i = 0; i < nodes.size(); i++) {
            auto& n = nodes[i];
            os << "    {\n";
            os << "      \"id\": " << n.id << ",\n";
            os << "      \"name\": \"" << jsonEscape(n.name) << "\",\n";

            if (n.type == NodeType::GROUP) os << "      \"type\": \"group\",\n";
            else if (n.type == NodeType::INSTANCE) os << "      \"type\": \"instance\",\n";
            else os << "      \"type\": \"sdf\",\n";

            os << "      \"mode\": \"" << (n.mode == NodeMode::VOLUME ? "volume" : "solid") << "\",\n";
            os << "      \"transform\": "; jsonTransform(os, n.transform, "      "); os << ",\n";

            if (!n.sdf_nodes.empty()) {
                os << "      \"sdf\": ";
                if (n.sdf_nodes.size() == 1) {
                    jsonSdf(os, n.sdf_nodes[0], "      ");
                } else {
                    os << "[\n";
                    for (size_t si = 0; si < n.sdf_nodes.size(); si++) {
                        os << "        ";
                        jsonSdf(os, n.sdf_nodes[si], "        ");
                        os << (si < n.sdf_nodes.size() - 1 ? "," : "") << "\n";
                    }
                    os << "      ]";
                }
                os << ",\n";
            }

            if (n.material_id) {
                os << "      \"material\": " << *n.material_id << ",\n";
            }

            os << "      \"density_scale\": " << n.density_scale << ",\n";
            os << "      \"blend_mode\": \"" << (n.blend_mode == BlendMode::ADD ? "add" : n.blend_mode == BlendMode::REPLACE ? "replace" : "alpha") << "\"";

            if (n.type == NodeType::GROUP && !n.children.empty()) {
                os << ",\n      \"children\": [";
                for (size_t j = 0; j < n.children.size(); j++) {
                    if (j > 0) os << ", ";
                    os << n.children[j];
                }
                os << "]";
            }

            if (n.type == NodeType::INSTANCE && n.def_id) {
                os << ",\n      \"def\": " << *n.def_id;
            }

            os << "\n    }" << (i < nodes.size() - 1 ? "," : "") << "\n";
        }
        os << "  ]";
    };

    if (!rih.defs.empty()) {
        writeNodeList(rih.defs, "defs");
        os << ",\n";
    }

    writeNodeList(rih.nodes, "nodes");
    os << "\n}\n";

    return os.str();
}

bool writeRihJson(const Rih& rih, const std::string& path) {
    std::ofstream file(path);
    if (!file) return false;
    file << rihToJsonString(rih);
    return file.good();
}

// ============================================================================
// Binary Writer (.rib)
// ============================================================================

bool writeRihBinary(const Rih& rih, const std::string& path) {
    std::ofstream file(path, std::ios::binary);
    if (!file) return false;

    // Build string table
    std::vector<std::string> strings;
    std::unordered_map<std::string, u32> string_offsets;
    auto addString = [&](const std::string& s) -> u32 {
        auto it = string_offsets.find(s);
        if (it != string_offsets.end()) return it->second;
        u32 offset = 0;
        for (auto& existing : strings) offset += (u32)existing.size() + 1;
        strings.push_back(s);
        string_offsets[s] = offset;
        return offset;
    };

    // Header
    binary::Header header;
    header.magic = binary::MAGIC;
    header.version = 1;
    header.scene_flags = (rih.scene.camera.fov > 0 ? 1 : 0);
    header.num_materials = (u32)rih.materials.size();
    header.num_nodes = (u32)rih.nodes.size();
    header.num_defs = (u32)rih.defs.size();
    header.num_lights = (u32)rih.lights.size();
    header.reserved = 0;
    file.write(reinterpret_cast<const char*>(&header), sizeof(header));

    // Scene
    file.write(reinterpret_cast<const char*>(&rih.scene.width), sizeof(u32));
    file.write(reinterpret_cast<const char*>(&rih.scene.height), sizeof(u32));
    file.write(reinterpret_cast<const char*>(&rih.scene.w_frames), sizeof(u32));
    file.write(reinterpret_cast<const char*>(&rih.scene.w_min), sizeof(f32));
    file.write(reinterpret_cast<const char*>(&rih.scene.w_max), sizeof(f32));

    // Camera
    file.write(reinterpret_cast<const char*>(&rih.scene.camera.position), sizeof(Vec3));
    file.write(reinterpret_cast<const char*>(&rih.scene.camera.target), sizeof(Vec3));
    file.write(reinterpret_cast<const char*>(&rih.scene.camera.up), sizeof(Vec3));
    file.write(reinterpret_cast<const char*>(&rih.scene.camera.fov), sizeof(f32));
    file.write(reinterpret_cast<const char*>(&rih.scene.camera.aperture), sizeof(f32));
    file.write(reinterpret_cast<const char*>(&rih.scene.camera.focal_dist), sizeof(f32));

    // Background
    file.write(reinterpret_cast<const char*>(&rih.scene.background), sizeof(ColorRGB));

    // Materials
    for (auto& m : rih.materials) {
        binary::MaterialRecord rec;
        rec.id = m.id;
        rec.name_offset = addString(m.name);
        rec.tensor_is_expr = 0;
        for (int i = 0; i < 8; i++) {
            if (m.tensor[i].is_expr) {
                rec.tensor[i].string_offset = addString(m.tensor[i].expression);
                rec.tensor_is_expr |= (1 << i);
            } else {
                rec.tensor[i].constant = m.tensor[i].constant;
            }
        }
        rec.blend_mode = (u8)m.blend_mode;
        memset(rec.padding, 0, sizeof(rec.padding));
        file.write(reinterpret_cast<const char*>(&rec), sizeof(rec));
    }

    // Lights
    for (auto& L : rih.lights) {
        binary::LightRecord rec;
        rec.id = L.id;
        rec.name_offset = addString(L.name);
        rec.light_type = (u8)L.type;
        rec.direction[0] = L.direction.x; rec.direction[1] = L.direction.y; rec.direction[2] = L.direction.z;
        rec.position[0] = L.position.x; rec.position[1] = L.position.y; rec.position[2] = L.position.z;
        rec.color[0] = L.color.r; rec.color[1] = L.color.g; rec.color[2] = L.color.b;
        rec.intensity = L.intensity;
        rec.falloff = L.falloff;
        memset(rec.padding, 0, sizeof(rec.padding));
        file.write(reinterpret_cast<const char*>(&rec), sizeof(rec));
    }

    auto writeNode = [&](const Node& n) {
        binary::NodeRecord rec;
        rec.id = n.id;
        rec.type = (u8)n.type;
        rec.mode = (u8)n.mode;
        rec.name_offset = addString(n.name);
        rec.material_id = n.material_id.value_or(0xFFFFFFFF);
        rec.density_scale = n.density_scale;
        rec.blend_mode = (u8)n.blend_mode;
        memset(rec.padding, 0, sizeof(rec.padding));
        file.write(reinterpret_cast<const char*>(&rec), sizeof(rec));

        // Transform
        for (int i = 0; i < 3; i++) {
            if (n.transform.translate[i].is_expr) {
                u32 off = addString(n.transform.translate[i].expression);
                file.write(reinterpret_cast<const char*>(&off), sizeof(u32));
            } else {
                file.write(reinterpret_cast<const char*>(&n.transform.translate[i].constant), sizeof(f32));
            }
        }
        for (int i = 0; i < 3; i++) {
            if (n.transform.rotate[i].is_expr) {
                u32 off = addString(n.transform.rotate[i].expression);
                file.write(reinterpret_cast<const char*>(&off), sizeof(u32));
            } else {
                file.write(reinterpret_cast<const char*>(&n.transform.rotate[i].constant), sizeof(f32));
            }
        }
        for (int i = 0; i < 3; i++) {
            if (n.transform.scale[i].is_expr) {
                u32 off = addString(n.transform.scale[i].expression);
                file.write(reinterpret_cast<const char*>(&off), sizeof(u32));
            } else {
                file.write(reinterpret_cast<const char*>(&n.transform.scale[i].constant), sizeof(f32));
            }
        }

        // SDF data
        u32 sdf_count = (u32)n.sdf_nodes.size();
        file.write(reinterpret_cast<const char*>(&sdf_count), sizeof(u32));
        for (auto& sdf : n.sdf_nodes) {
            binary::SdfRecord srec;
            srec.sdf_type = (u8)sdf.type;
            for (int i = 0; i < 4; i++) {
                if (sdf.params[i].is_expr) {
                    srec.params[i].string_offset = addString(sdf.params[i].expression);
                } else {
                    srec.params[i].constant = sdf.params[i].constant;
                }
            }
            srec.child_a = sdf.child_a.value_or(0xFFFFFFFF);
            srec.child_b = sdf.child_b.value_or(0xFFFFFFFF);
            srec.k = sdf.k.value_or(0.0f);
            srec.spacing = sdf.spacing.value_or(0.0f);
            srec.displace_expr_offset = sdf.displace_func ? addString(*sdf.displace_func) : 0xFFFFFFFF;
            file.write(reinterpret_cast<const char*>(&srec), sizeof(srec));
        }

        // Children (group)
        if (n.type == NodeType::GROUP) {
            u32 num = (u32)n.children.size();
            file.write(reinterpret_cast<const char*>(&num), sizeof(u32));
            file.write(reinterpret_cast<const char*>(n.children.data()), num * sizeof(u32));
        }

        // Def id (instance)
        if (n.type == NodeType::INSTANCE) {
            u32 def = n.def_id.value_or(0xFFFFFFFF);
            file.write(reinterpret_cast<const char*>(&def), sizeof(u32));
        }
    };

    // Write defs
    for (auto& n : rih.defs) writeNode(n);

    // Write nodes
    for (auto& n : rih.nodes) writeNode(n);

    // String table
    for (auto& s : strings) {
        file.write(s.c_str(), s.size() + 1);
    }

    return file.good();
}

// ============================================================================
// Frame evaluation
// ============================================================================

static void evalTransform(Transform& t, float frame) {
    for (int i = 0; i < 3; i++) {
        if (t.translate[i].is_expr) {
            t.translate[i].constant = evalExpression(t.translate[i].expression, frame);
            t.translate[i].is_expr = false;
        }
        if (t.rotate[i].is_expr) {
            t.rotate[i].constant = evalExpression(t.rotate[i].expression, frame);
            t.rotate[i].is_expr = false;
        }
        if (t.scale[i].is_expr) {
            t.scale[i].constant = evalExpression(t.scale[i].expression, frame);
            t.scale[i].is_expr = false;
        }
    }
}

static void evalSdfExprParams(SdfNode& sdf, float frame) {
    for (int i = 0; i < 4; i++) {
        if (sdf.params[i].is_expr) {
            sdf.params[i].constant = evalExpression(sdf.params[i].expression, frame);
            sdf.params[i].is_expr = false;
        }
    }
}

static void evalNodeSdfParams(Node& n, float frame) {
    for (auto& sdf : n.sdf_nodes) {
        evalSdfExprParams(sdf, frame);
    }
}

Rih evaluateFrame(const Rih& rih, float t, bool fold_sdf_params) {
    Rih out = rih;

    for (auto& n : out.nodes) {
        evalTransform(n.transform, t);
        if (fold_sdf_params) evalNodeSdfParams(n, t);
    }
    for (auto& n : out.defs) {
        evalTransform(n.transform, t);
        if (fold_sdf_params) evalNodeSdfParams(n, t);
    }

    // Update scene time range for single frame
    out.scene.w_frames = 1;
    out.scene.w_min = t;
    out.scene.w_max = t;

    return out;
}

std::string framePath(const std::string& prefix, int frame, bool binary) {
    std::ostringstream os;
    os << prefix;
    os << std::setw(4) << std::setfill('0') << frame;
    os << (binary ? ".rib" : ".rih");
    return os.str();
}

} // namespace herm
