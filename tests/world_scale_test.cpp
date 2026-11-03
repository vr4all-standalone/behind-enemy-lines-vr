#include "../native/settings_io.h"
#include "../native/controller_controls.h"
#include "../native/projection.h"
#include <cassert>
#include <sstream>
using namespace bel;
int main() {
    Settings s;
    assert(s.panel_distance==1.8f&&s.view_fov==65&&s.world_scale==.1f);
    std::istringstream saved_choice("version=6\nwide_world_scale=0.6\npanel_distance=2.4\nview_fov=75\n");
    auto chosen=read_settings(saved_choice);assert(chosen.wide_world_scale==.6f&&chosen.panel_distance==2.4f&&chosen.view_fov==75);
    assert(active_world_scale(s)==.830f);
    s.wider_view=false;assert(active_world_scale(s)==.830f);
    active_world_scale(s)=.14f;s.wider_view=true;
    assert(active_world_scale(s)==.14f);
    active_world_scale(s)=.42f;
    std::ostringstream out;write_settings(out,s);std::istringstream in(out.str());
    auto loaded=read_settings(in);assert(active_world_scale(loaded)==.42f);
    loaded.wider_view=false;assert(active_world_scale(loaded)==.42f);
    std::istringstream old_default("version=4\ngun_angle_zero_version=1\nworld_scale=0.1\nwider_view=0\n");
    bool migrated=false;auto old=read_settings(old_default,&migrated);
    assert(migrated&&!old.wider_view&&active_world_scale(old)==.1f);
    old.wider_view=true;assert(active_world_scale(old)==.1f);
    std::istringstream old_choice("version=4\nworld_scale=0.25\n");
    old=read_settings(old_choice);assert(active_world_scale(old)==.25f);
    old.wider_view=false;assert(active_world_scale(old)==.25f);
    std::istringstream malformed("version=5\nwide_world_scale=nan\nworld_scale=0.1\n");
    assert(active_world_scale(read_settings(malformed))==.830f);
    std::istringstream prior_wide("version=5\nwide_world_scale=0.3\nworld_scale=0.1\n");
    assert(active_world_scale(read_settings(prior_wide))==.830f);
    // Metric scale changes world dimensions, preserving the original central
    // projection. Screen-space panels remain at their own physical distance.
    NativeLens lens{496,384,400,400,249,192};
    auto a=lens.unproject(320,160,10*.1),b=lens.unproject(320,160,10*.3);
    assert(std::abs(b.x/a.x-3)<1e-6&&std::abs(b.z/a.z-3)<1e-6);
    assert(std::abs(lens.project_point(a).u-lens.project_point(b).u)<1e-8);
}
