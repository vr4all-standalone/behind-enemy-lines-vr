#include "../native/reticle_size.h"
#include "../native/reticle_mode.h"
#include "../native/custom_reticle.h"
#include <cassert>
#include <cmath>
using namespace bel;
int main(){
    const ReticleSize natural{.25f,1.7f};
    assert(reticle_size_reference(1,1.7,natural)==1.7);
    assert(std::abs(reticle_size_reference(4,1.7,natural)-1.7)<1e-6);
    assert(std::abs(reticle_size_reference(20,1.7,natural)/20-.25)<1e-6);
    assert(std::abs(reticle_size_reference(40,1.7,natural)/40-.25)<1e-6);
    assert(reticle_size_reference(10,1.7,{.25f,10})>reticle_size_reference(10,1.7,natural));
    assert(reticle_size_reference(20,1.7,{.5f,1.7f})==10);
    assert(reticle_size_settings(0,100).minimum_ratio==.05f&&reticle_size_settings(0,100).shrink_distance==20);
    assert(next_reticle_mode(ReticleMode::None,1)==ReticleMode::Ingame);
    assert(next_reticle_mode(ReticleMode::Ingame,-1)==ReticleMode::None);
    assert(reticle_mode_value(99)==ReticleMode::Ingame);
    auto custom=custom_reticle_mesh();assert(custom.size()==72);
    for(const auto& v:custom)assert(std::abs(v.x)>.1&&std::abs(v.y)>.1); // Empty aiming center.
}
