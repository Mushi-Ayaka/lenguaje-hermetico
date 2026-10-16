#include "herm_obs.h"
#include <fstream>
#include <vector>
#include <cstring>

namespace herm {

static std::string obsPathFromOnt(const std::string& ont_path) {
    auto dot = ont_path.rfind('.');
    auto slash = ont_path.find_last_of("/\\");
    if (dot != std::string::npos && dot != 0 &&
        (slash == std::string::npos || dot > slash)) {
        return ont_path.substr(0, dot) + ".obs";
    }
    return ont_path + ".obs";
}

static uint32_t align4(uint32_t offset) {
    return (offset + 3) & ~3u;
}

bool writeObs(const Rih& rih, const std::string& ont_path) {
    std::string obs_path = obsPathFromOnt(ont_path);
    std::ofstream out(obs_path, std::ios::binary);
    if (!out) return false;

    uint32_t flags = 0;
    uint32_t camera_off = 0, lights_off = 0, timeline_off = 0;
    uint32_t bg_off = 0, res_off = 0;

    // Pre-compute offsets (header is 64 bytes)
    uint32_t cur = sizeof(ObsHeader);

    // Camera
    bool has_camera = true; // always write camera from RIH
    if (has_camera) {
        flags |= OBS_CAMERA;
        camera_off = cur;
        cur += sizeof(ObsCamera);
    }

    // Lights
    bool has_lights = !rih.lights.empty();
    if (has_lights) {
        flags |= OBS_LIGHTS;
        lights_off = cur;
        cur += sizeof(ObsLightsHeader) + (uint32_t)rih.lights.size() * sizeof(ObsLight);
    }

    // Timeline
    bool has_timeline = (rih.scene.w_frames > 1);
    if (has_timeline) {
        flags |= OBS_TIMELINE;
        timeline_off = cur;
        cur += sizeof(ObsTimeline);
        if (!rih.scene.t_unit.empty()) {
            cur += (uint32_t)rih.scene.t_unit.size();
        }
        cur = align4(cur);
    }

    // Background
    bool has_background = true;
    if (has_background) {
        flags |= OBS_BACKGROUND;
        bg_off = cur;
        cur += 12; // 3 floats
    }

    // Resolution
    bool has_resolution = true;
    if (has_resolution) {
        flags |= OBS_RESOLUTION;
        res_off = cur;
        cur += 8; // 2 uint32_t
    }

    // --- Write Header ---
    ObsHeader hdr;
    std::memset(&hdr, 0, sizeof(hdr));
    hdr.magic = OBS_MAGIC;
    hdr.version = 1;
    hdr.flags = flags;
    hdr.camera_offset = camera_off;
    hdr.lights_offset = lights_off;
    hdr.timeline_offset = timeline_off;
    hdr.background_offset = bg_off;
    hdr.resolution_offset = res_off;
    out.write(reinterpret_cast<const char*>(&hdr), sizeof(hdr));

    // --- Camera ---
    ObsCamera cam;
    cam.position[0] = rih.scene.camera.position.x;
    cam.position[1] = rih.scene.camera.position.y;
    cam.position[2] = rih.scene.camera.position.z;
    cam.target[0] = rih.scene.camera.target.x;
    cam.target[1] = rih.scene.camera.target.y;
    cam.target[2] = rih.scene.camera.target.z;
    cam.up[0] = rih.scene.camera.up.x;
    cam.up[1] = rih.scene.camera.up.y;
    cam.up[2] = rih.scene.camera.up.z;
    cam.fov = rih.scene.camera.fov;
    cam.aperture = rih.scene.camera.aperture;
    cam.focal_dist = rih.scene.camera.focal_dist;
    out.write(reinterpret_cast<const char*>(&cam), sizeof(cam));

    // --- Lights ---
    if (has_lights) {
        ObsLightsHeader lhdr;
        lhdr.count = (uint32_t)rih.lights.size();
        out.write(reinterpret_cast<const char*>(&lhdr), sizeof(lhdr));
        for (auto& l : rih.lights) {
            ObsLight ol;
            std::memset(&ol, 0, sizeof(ol));
            ol.type = (l.type == LightType::POINT) ? OBS_LIGHT_POINT : OBS_LIGHT_DIRECTIONAL;
            ol.direction[0] = l.direction.x;
            ol.direction[1] = l.direction.y;
            ol.direction[2] = l.direction.z;
            ol.position[0] = l.position.x;
            ol.position[1] = l.position.y;
            ol.position[2] = l.position.z;
            ol.color[0] = l.color.r;
            ol.color[1] = l.color.g;
            ol.color[2] = l.color.b;
            ol.intensity = l.intensity;
            ol.falloff = l.falloff;
            out.write(reinterpret_cast<const char*>(&ol), sizeof(ol));
        }
    }

    // --- Timeline ---
    if (has_timeline) {
        ObsTimeline tl;
        tl.w_frames = rih.scene.w_frames;
        tl.w_min = rih.scene.w_min;
        tl.w_max = rih.scene.w_max;
        if (rih.scene.t_unit.empty()) {
            tl.t_unit_len = 0;
        } else {
            tl.t_unit_len = (uint32_t)rih.scene.t_unit.size();
        }
        out.write(reinterpret_cast<const char*>(&tl), sizeof(tl));
        if (tl.t_unit_len > 0) {
            out.write(rih.scene.t_unit.data(), tl.t_unit_len);
        }
        // align to 4 bytes
        uint32_t written = sizeof(tl) + tl.t_unit_len;
        uint32_t pad = (4 - (written % 4)) % 4;
        if (pad) {
            uint8_t zero = 0;
            for (uint32_t i = 0; i < pad; i++) out.write(reinterpret_cast<const char*>(&zero), 1);
        }
    }

    // --- Background ---
    float bg[3] = { rih.scene.background.r, rih.scene.background.g, rih.scene.background.b };
    out.write(reinterpret_cast<const char*>(bg), sizeof(bg));

    // --- Resolution ---
    uint32_t res[2] = { rih.scene.width, rih.scene.height };
    out.write(reinterpret_cast<const char*>(res), sizeof(res));

    return out.good();
}

} // namespace herm
