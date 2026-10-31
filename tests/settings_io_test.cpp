#include "../native/settings_io.h"
#include <cassert>
#include <sstream>
using namespace bel;
int main(){
    assert(Settings{}.p1_hand==0);
    assert(Settings{}.wider_view);
    std::ostringstream accepted_defaults;write_settings(accepted_defaults,Settings{});
    assert(accepted_defaults.str().find("wide_world_scale=0.83\n")!=std::string::npos);
    assert(accepted_defaults.str().find("panel_distance=1.8\n")!=std::string::npos);
    assert(accepted_defaults.str().find("reticle_min_size=0.35\n")!=std::string::npos);
    assert(accepted_defaults.str().find("reticle_max_size=1.45\n")!=std::string::npos);
    assert(accepted_defaults.str().find("reticle_shrink_distance=10\n")!=std::string::npos);
    assert(accepted_defaults.str().find("hud_size=1.2\n")!=std::string::npos);
    std::istringstream legacy("version=1\nwider_view=0\nworld_scale=0.25\n");
    auto migrated=read_settings(legacy);
    assert(migrated.wider_view && migrated.world_scale==.25f);
    std::istringstream fresh("laser=0\n");
    assert(read_settings(fresh).wider_view);
    std::istringstream current("wider_view=0\nversion=2\n");
    auto selected=read_settings(current);assert(!selected.wider_view);
    std::ostringstream output;write_settings(output,selected);
    assert(output.str().find("version=7\n")==0);
    std::istringstream roundtrip(output.str());assert(!read_settings(roundtrip).wider_view);
    std::istringstream invalid("version=2\nwider_view=nan\n");
    assert(read_settings(invalid).wider_view);
    std::istringstream old_hands("version=2\np1_hand=1\nwider_view=0\nworld_scale=0.25\n");
    auto hand_migration=read_settings(old_hands);
    assert(hand_migration.p1_hand==0&&!hand_migration.wider_view&&hand_migration.world_scale==.25f);
    // Once upgraded, an explicit reversed assignment survives later launches.
    hand_migration.p1_hand=1;
    std::ostringstream reversed;write_settings(reversed,hand_migration);
    std::istringstream reloaded(reversed.str());
    assert(read_settings(reloaded).p1_hand==1);
    std::istringstream current_hands("p1_hand=1\nversion=7\ngun_angle_zero_version=1\n");
    bool upgraded=true;assert(read_settings(current_hands,&upgraded).p1_hand==1&&!upgraded);
    std::istringstream needs_upgrade("version=2\np1_hand=1\n");
    assert(read_settings(needs_upgrade,&upgraded).p1_hand==0&&upgraded);
    // Displayed zero now means the agreed physical -10-degree baseline.
    std::istringstream old_angle("version=3\ngun_angle=17\nlaser=0\n");
    auto angle=read_settings(old_angle,&upgraded);
    assert(angle.gun_angle==0 && !angle.laser && upgraded);
    angle.gun_angle=12;
    std::ostringstream angle_saved;write_settings(angle_saved,angle);
    std::istringstream angle_loaded(angle_saved.str());
    assert(read_settings(angle_loaded,&upgraded).gun_angle==12&&!upgraded);
    std::istringstream old_view("version=6\ngun_angle_zero_version=1\nwide_world_scale=0.815\nworld_scale=0.14\npanel_distance=2.3\nview_fov=75\n");
    auto view=read_settings(old_view,&upgraded);assert(upgraded&&view.reticle_shrink_distance==2.3f&&view.world_scale==.14f&&view.wide_world_scale==.815f&&view.view_fov==75);
    view.reticle_mode=ReticleMode::Custom;view.reticle_min_size=.4f;view.reticle_shrink_distance=6.2f;
    std::ostringstream reticle_saved;write_settings(reticle_saved,view);std::istringstream reticle_loaded(reticle_saved.str());
    auto stored=read_settings(reticle_loaded,&upgraded);assert(!upgraded&&stored.reticle_mode==ReticleMode::Custom&&stored.reticle_min_size==.4f&&stored.reticle_shrink_distance==6.2f);
    std::istringstream malformed("version=7\nreticle_mode=99\nreticle_min_size=nan\nreticle_shrink_distance=-1\n");
    auto safe=read_settings(malformed);assert(safe.reticle_mode==ReticleMode::Ingame&&safe.reticle_min_size==.35f&&safe.reticle_shrink_distance==10.f);
    assert(Settings{}.hud_size==1.2f);
    for(const char* text:{"hud_size=nan\n","hud_size=inf\n","hud_size=0.49\n","hud_size=2.01\n","hud_size=no\n",""}){
        std::istringstream source(text);assert(read_settings(source).hud_size==1.2f);
    }
    for(float size:{.5f,1.f,2.f}){
        auto chosen=stored;chosen.hud_size=size;chosen.wider_view=false;
        std::ostringstream saved;write_settings(saved,chosen);std::istringstream source(saved.str());auto loaded=read_settings(source);
        assert(loaded.hud_size==size&&!loaded.wider_view&&loaded.reticle_mode==chosen.reticle_mode&&loaded.reticle_min_size==chosen.reticle_min_size);
        loaded.wider_view=true;assert(loaded.hud_size==size); // Mode switches never replace the saved choice.
    }
}
