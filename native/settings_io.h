// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include "input_policy.h"
#include <istream>
#include <ostream>
#include <sstream>
#include <string>
namespace bel {
inline Settings read_settings(std::istream& input,bool* migrated=nullptr) {
    bool has_custom_min=false,has_ingame_min=false,has_panel_distance=false;
    Settings out;std::string line;int version{};bool angle_zero_version{},has_wider{},selected_wider{},has_hand{},has_wide_scale{},has_reticle_distance{};int selected_hand{};
    while(std::getline(input,line)) {
        auto equals=line.find('=');if(equals==std::string::npos)continue;
        auto key=line.substr(0,equals);std::istringstream value(line.substr(equals+1));double number{};char extra{};
        if(!(value>>number)||(value>>extra)||!std::isfinite(number))continue;
        auto integer=[&](int lo,int hi){return number==std::floor(number)&&number>=lo&&number<=hi;};
        if(key=="version"&&integer(1,1000000))version=int(number);
        else if(key=="gun_angle_zero_version"&&integer(1,2147483647))angle_zero_version=true;
        else if(key=="p1_hand"&&integer(0,1)){has_hand=true;selected_hand=int(number);}
        else if(key=="gun"&&integer(0,1))out.gun=int(number);
        else if(key=="laser"&&integer(0,1))out.laser=number!=0;
        else if(key=="laser_width"&&integer(1,3))out.laser_width=number;
        else if(key=="wider_view"&&integer(0,1)){has_wider=true;selected_wider=number!=0;}
        else if(key=="background"&&integer(0,1))out.background=BackgroundMode(int(number));
        else if(key=="view_fov"&&number>=35&&number<=95)out.view_fov=float(number);
        else if(key=="world_scale"&&number>=.01&&number<=1)out.world_scale=float(number);
        else if(key=="wide_world_scale"&&number>=.01&&number<=1){out.wide_world_scale=float(number);has_wide_scale=true;}
        else if(key=="panel_distance"&&number>=.5&&number<=10){out.panel_distance=float(number);has_panel_distance=true;}
        else if(key=="hud_size"&&number>=.5&&number<=2)out.hud_size=float(number);
        else if(key=="reticle_mode"&&integer(0,2))out.reticle_mode=reticle_mode_value(int(number));
        else if(key=="reticle_min_size"&&number>=.05&&number<=1){out.reticle_min_size=float(number);has_ingame_min=true;}
        else if(key=="reticle_max_size"&&number>=.25&&number<=3)out.reticle_max_size=float(number);
        else if(key=="reticle_custom_min_size"&&number>=.05&&number<=1){out.reticle_custom_min_size=float(number);has_custom_min=true;}
        else if(key=="reticle_custom_max_size"&&number>=.25&&number<=3)out.reticle_custom_max_size=float(number);
        else if(key=="reticle_shrink_distance"&&number>=.2&&number<=20){out.reticle_shrink_distance=float(number);has_reticle_distance=true;}
        else if(key=="gun_angle"&&number>=-30&&number<=30)out.gun_angle=float(number);
        else if(key=="gun_scale"&&number>=.4&&number<=1.6)out.gun_scale=float(number);
        else if(key=="haptics"&&number>=0&&number<=1)out.haptics=HapticLevels[haptic_level(float(number))];
    }
    // Older builds started with a narrow view. Apply the requested new default
    // once; version 2 retains subsequent explicit user choices, including Off.
    if(version>=2&&has_wider)out.wider_view=selected_wider;
    // Version 3 changes the old right-hand default once. Later explicit hand
    // reversals retain their saved value, independently of view migration.
    if(version>=3&&has_hand)out.p1_hand=selected_hand;
    if(!angle_zero_version)out.gun_angle=0;
    // Keep the user's screen mode and non-default legacy size choices. Only
    // the old default receives the new, larger Wider-view starting scale.
    if(!has_wide_scale&&out.world_scale!=.1f)out.wide_world_scale=out.world_scale;
    if(version<6&&out.wide_world_scale==.3f)out.wide_world_scale=Settings{}.wide_world_scale;
    if(!has_reticle_distance&&version<7&&has_panel_distance)out.reticle_shrink_distance=out.panel_distance;
    if(version<8&&!out.wider_view)out.wide_world_scale=out.world_scale;
    if(migrated)*migrated=version<8||!angle_zero_version;
    if(!has_custom_min&&has_ingame_min)out.reticle_custom_min_size=out.reticle_min_size;
    return out;
}
inline void write_settings(std::ostream& out,const Settings& s) {
    out<<"version=8\ngun_angle_zero_version=1\np1_hand="<<s.p1_hand<<"\ngun="<<(s.gun==1?1:0)<<"\nlaser="<<s.laser
       <<"\nlaser_width="<<s.laser_width<<"\nview_fov="<<s.view_fov<<"\nwider_view="<<s.wider_view<<"\nworld_scale="<<s.world_scale<<"\nwide_world_scale="<<s.wide_world_scale<<"\npanel_distance="<<s.panel_distance<<"\nhud_size="<<s.hud_size<<"\ngun_angle="<<s.gun_angle<<"\ngun_scale="<<s.gun_scale<<"\nreticle_mode="<<int(s.reticle_mode)<<"\nreticle_min_size="<<s.reticle_min_size<<"\nreticle_max_size="<<s.reticle_max_size<<"\nreticle_custom_min_size="<<s.reticle_custom_min_size<<"\nreticle_custom_max_size="<<s.reticle_custom_max_size<<"\nreticle_shrink_distance="<<s.reticle_shrink_distance<<"\nhaptics="<<s.haptics<<"\nbackground="<<int(s.background)<<'\n';
}
}
