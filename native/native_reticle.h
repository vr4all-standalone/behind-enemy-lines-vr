// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include "model2_scene.h"
namespace bel {
struct ReticleVertex {float x{},y{};uint32_t color{};bool operator==(const ReticleVertex&)const=default;};
using NativeReticle=std::vector<ReticleVertex>;
// Recover the original authored sight from the live native HUD geometry.
// The inverse object transform removes native cabinet aim/animation placement.
// Long ruler/grid polygons extend to +/-19.37 local units; the complete
// circular sight and its short ticks fit within +/-0.8. Do not include rulers.
inline NativeReticle native_reticle(const NativeFrame& frame,const Model2Resources& resources){
    NativeReticle out;if(!frame.complete||frame.rejected||!resources.valid())return out;
    std::optional<uint32_t> object;
    for(const auto& p:frame.polygons){
        if(!native_aim_grid(p)||p.culled||p.count<3||p.count>4||p.header[0]!=0)continue;
        if(object&&*object!=p.object)continue;
        const auto& m=p.matrix;
        Vec3 a{m[0],m[1],m[2]},b{m[3],m[4],m[5]},c{m[6],m[7],m[8]};
        const auto bc=cross(b,c),ca=cross(c,a),ab=cross(a,b);const double det=dot(a,bc);
        if(!std::isfinite(det)||std::abs(det)<1e-9)continue;
        std::array<Vec3,4> local;bool valid=true;
        for(unsigned i=0;i<p.count;i++){
            const auto& v=p.vertices[i];Vec3 q{v.x/p.focus[0]-m[9],v.y/p.focus[1]-m[10],v.z-m[11]};
            local[i]={dot(q,bc)/det,dot(q,ca)/det,dot(q,ab)/det};
            valid=valid&&finite(local[i])&&std::abs(local[i].x)<=1&&std::abs(local[i].y)<=1&&std::abs(local[i].z)<.02;
        }
        if(!valid)continue;
        auto color=resources.sample(p,0,0,p.vertices[0].z,0,0);if(!color)continue;
        object=p.object;
        auto vertex=[&](unsigned i){return ReticleVertex{float(local[i].x),float(local[i].y),*color};};
        for(unsigned i=1;i+1<p.count;i++)for(unsigned j:{0u,i,i+1})out.push_back(vertex(j));
    }
    return out;
}
}
