// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include "model2_capture.h"
#include <algorithm>
namespace bel {
struct HudRect {float left,top,right,bottom;};
inline std::array<HudRect,6> essential_hud_regions(int width,int height) {
    const float x=width/496.f,y=height/384.f;
    return {{{24*x,76*y,56*x,272*y},{440*x,76*y,472*x,272*y},
        {184*x,0,313*x,62*y},{24*x,272*y,472*x,376*y},
        {0,0,76*x,54*y},{420*x,0,496*x,54*y}}};
}
inline bool essential_hud_pixel(int x,int y,int width,int height) {
    for(auto r:essential_hud_regions(width,height))
        if(x>=r.left&&x<r.right&&y>=r.top&&y<r.bottom)return true;
    return false;
}
// Original BEL captures identify window4 as the HUD pass. Its focus400
// objects are the two aiming meshes and their extended rulers; focus330 supplies the
// original status indicators. Never use either as physical aiming surfaces.
inline bool native_hud_geometry(const NativePolygon& p){return p.window==4;}
inline bool native_aim_grid(const NativePolygon& p){return native_hud_geometry(p)&&p.focus[0]==400.f&&p.focus[1]==400.f;}
inline std::vector<uint32_t> essential_hud_raster(const std::vector<uint32_t>& source,int width,int height) {
    if(width<=0||height<=0||source.size()!=size_t(width)*height)return source;
    std::vector<uint32_t> result(source.size());
    for(auto r:essential_hud_regions(width,height)) {
        const int left=std::clamp(int(std::ceil(r.left)),0,width),right=std::clamp(int(std::ceil(r.right)),0,width);
        for(int y=std::max(0,int(std::ceil(r.top)));y<std::min(height,int(std::ceil(r.bottom)));++y)
            std::copy_n(source.begin()+size_t(y)*width+left,right-left,result.begin()+size_t(y)*width+left);
    }
    return result;
}
}
