// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include "model2_scene.h"
#include "native_reticle.h"
#include <glm/glm.hpp>
#include <memory>
namespace bel {
class GLScene {
    struct Impl;std::unique_ptr<Impl> impl_;
public:
    GLScene();~GLScene();
    // Optional camera-space prototype. Native/reference presentation is default.
    void set_immersive(bool enabled);
    void set_gameplay_hud(bool minimal);
    void prepare(std::shared_ptr<const NativeFrame> frame,bool wider_view=false);
    void set_presentation_lens(const NativeLens& lens);
    SceneAim trace_aim(Ray ray,float world_scale,float panel_distance) const;
    std::shared_ptr<const NativeReticle> reticle() const;
    void draw(const glm::mat4& view_projection,float world_scale,float panel_distance,float hud_size=1.f);
};
}
