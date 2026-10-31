// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include "cabinet_io.h"
#include <array>
#include <bit>
#include <cstdint>
namespace bel {
struct PlayerState {
    uint32_t state{},ammo{},capacity{},shots{},weapon{};
    float health{};
    bool joined() const {return state!=0;}
    bool alive() const {return joined()&&health>0;}
};
struct GameState {
    bool valid{},ready{},playing{},counting_shots{},service{},gun_calibration{};
    bool shot_counters_valid{};
    bool calibration_erased{},calibration_config_valid{},automatic_calibration{};
    uint32_t service_selection{};
    uint32_t mode{},submode{},service_menu{},service_submode{},service_busy{};
    uint64_t session{};
    std::array<PlayerState,2> players{};
    bool credits_valid{},free_play{};
    uint32_t credits{},start_cost{},continue_cost{};
};
// Exact original BEL CPU instruction/address words. These identify its input
// decoder, native mode dispatcher, player activation stores and service entry/
// exit paths before any RAM state is trusted. See native-state-evidence.md.
template<class Read32> GameState read_game_state(Read32 read) {
    GameState out;
    constexpr std::array<std::array<uint32_t,2>,12> fingerprint{{
        {0x56690,0x80803000},{0x56694,0x005fa12c},
        {0x566ac,0x90903903},{0x566b0,0x029767f0},
        {0x513e0,0x8ca03000},{0x513e4,0x01c00002},
        {0x5d150,0xca91a05a},{0x5d154,0xca91a056},
        {0x6957c,0x82883000},{0x69580,0x005a8831},
        {0x12a4,0x82b03000},{0x12a8,0x005a8831}
    }};
    for(const auto& word:fingerprint)if(read(word[0])!=word[1])return out;
    const auto byte=[&](uint32_t a){return uint8_t(read(a&~3u)>>((a&3)*8));};
    const auto half=[&](uint32_t a){return int16_t(read(a&~3u)>>((a&2)*8));};
    const auto execution=byte(0x5a8831);
    if((execution!=1&&execution!=0xff)||read(0x5a85e8)!=0x5a85f0)return out;
    out.valid=true;out.mode=byte(0x5fa12c);out.submode=byte(0x5b4b5c);
    out.calibration_erased=true;out.calibration_config_valid=byte(0x5a8604)==1;
    for(unsigned a=0x5a8607;a<=0x5a860e;a++)out.calibration_erased&=byte(a)==0xff;
    for(unsigned p=0;p<2;p++) {
        const unsigned x=byte(0x5a8607+p),y=byte(0x5a8609+p);
        const unsigned dx=byte(0x5a860b+p),dy=byte(0x5a860d+p);
        out.calibration_config_valid&=dx&&dy&&x+dx<=255&&y+dy<=255;
    }
    out.service=execution==0xff;
    out.credits_valid=true;out.credits=byte(0x5a87f0);
    out.free_play=byte(0x5a8810)!=0;
    out.start_cost=byte(0x5a85ff);out.continue_cost=byte(0x5a8600);
    // Native 5d0a0 initializes one independent signed lifetime at each 0x84
    // player stride. Start/credit traces prove positive=active; the native
    // credit code distinguishes negative continue state from inactive zero.
    for(unsigned player=0;player<2;player++) {
        const auto lifetime=half(0x5b4996+0x84*player);
        out.players[player].state=lifetime>0?1:lifetime<0?2:0;
        // Availability only: do not present this as a measured health gauge.
        out.players[player].health=lifetime>0?1.f:0.f;
    }
    // Observed title, demo and live-world handlers. Native introduction states
    // remain distinguishable and do not authorize new gameplay firing.
    out.playing=!out.service&&out.mode==21;
    out.ready=!out.service&&((out.mode==4&&out.submode==0)||(out.mode==10&&out.submode==1)||out.mode==21);
    // Continue remains native coin/Start, never a forced life/state write.
    // Quest death logs and original cabinet-input traces reach these states;
    // their original handlers accept Start while a player's lifetime is negative.
    const bool dead=out.players[0].state==2||out.players[1].state==2;
    const bool continue_handler=(out.mode==23&&read(0x59a88)==0x90903000&&read(0x59a8c)==0x005a0cfc)||
        (out.mode==27&&read(0x5a310)==0x90803000&&read(0x5a314)==0x005a0cfc)||
        (out.mode==29&&read(0x596c8)==0x0900d858);
    out.ready|=!out.service&&dead&&continue_handler;
    if(out.service) {
        out.service_selection=byte(0x5a883c);
        out.service_menu=byte(0x5a883d);
        out.service_submode=byte(0x5b4b64);
        out.gun_calibration=out.service_menu==3;
        if(out.gun_calibration&&out.service_submode>=3&&out.service_submode<=10)
            out.service_busy=out.service_submode<7?1:2;
    }
    return out;
}
class ShotTracker {
public:
    void suspend() {}
    template<class Read32> void update(GameState& state,Read32 read) {
        state.counting_shots=state.shot_counters_valid=false;
        for(auto& player:state.players)player.shots=0;
        if(!state.valid||!state.playing||state.service)return;
        // Original main-gun firing gate -> projectile allocation -> cumulative
        // fire count. Independent P1/P2 traces pair increments with newly
        // allocated projectiles and their native owner; this is not trigger state.
        constexpr std::array<std::array<uint32_t,2>,6> fingerprint{{
            {0x60bc8,0x350360bc},{0x60c00,0x09fb7c80},{0x60c78,0x90b8e024},
            {0x60c7c,0x59bdc801},{0x60c80,0x92b8e024},{0x188d4,0x8240e030}}};
        for(auto word:fingerprint)if(read(word[0])!=word[1])return;
        state.counting_shots=state.shot_counters_valid=true;
        for(unsigned p=0;p<2;p++)state.players[p].shots=read(0x5b4964+p*0x84);
    }
};
struct NativeAimCalibration {
    bool valid{},factory{},screen_inverse_valid{};
    uint8_t left{},right{},top{},bottom{},center_x{},center_y{};
    float angle_units_x{},angle_units_y{},origin_x{},origin_y{};
    // Measured BEL camera-space point convention, not projectile ownership.
    float yaw_scale{},previous_yaw{},minimum_pitch{},point_distance{};
};
template<class Read32> NativeAimCalibration read_native_calibration(Read32 read,unsigned player) {
    NativeAimCalibration out;
    if(player>1)return out;
    const auto state=read_game_state(read);
    if(!state.valid||!state.playing)return out;
    constexpr std::array<std::array<uint32_t,2>,8> converter{{
        {0x632f0,0x80803000},{0x632f4,0x005a8607},
        {0x63738,0x90483904},{0x6373c,0x005a0224},
        {0x63b98,0x8ca83000},{0x63b9c,0x4091f2e5},
        {0x63c4c,0x8ca03000},{0x63c50,0xbe1ef73c}
    }};
    for(const auto& word:converter)if(read(word[0])!=word[1])return out;
    const auto byte=[&](uint32_t a){return uint8_t(read(a&~3u)>>((a&3)*8));};
    const auto half=[&](uint32_t a){return int16_t(read(a&~3u)>>((a&2)*8));};
    const auto real=[&](uint32_t a){return std::bit_cast<float>(read(a));};
    if(byte(0x5a8604)!=1)return out; // Actual native analog-gun mode.
    const unsigned xmin=byte(0x5a8607+player),ymin=byte(0x5a8609+player);
    const unsigned dx=byte(0x5a860b+player),dy=byte(0x5a860d+player);
    if(!dx||!dy||xmin+dx>255||ymin+dy>255)return out;
    // Require the original converter to have applied the saved EEPROM values.
    if(real(0x5a17d4+player*4)!=float(xmin)||real(0x5a2230+player*4)!=float(ymin)||
       real(0x5a021c+player*4)!=dx*.5f||real(0x5a0224+player*4)!=dy*.5f)return out;
    // This observed live-world lens has 7,762 native point samples. Other
    // camera modes remain closed until their conversion is independently read.
    const float focus=real(0x5b489c),scale=real(0x5b4aa8);
    const int xlimit=half(0x5b4a80),ybase=half(0x5b4b1c),yfloor=half(0x5b4b1a);
    const int previous=half(0x5b4984+player*0x84);
    if(focus!=400.f||!std::isfinite(scale)||std::abs(scale-.428571462631f)>1e-7f||
       xlimit!=5400||ybase!=3540||yfloor!=-2476||std::abs(previous)>xlimit)return out;
    out.valid=true;out.left=xmin;out.right=xmin+dx;out.bottom=ymin;out.top=ymin+dy;
    out.center_x=uint8_t(xmin+(dx+1)/2);out.center_y=uint8_t(ymin+(dy+1)/2);
    out.angle_units_x=float(xlimit);out.angle_units_y=float(ybase);
    out.yaw_scale=scale;out.previous_yaw=float(previous);out.minimum_pitch=float(yfloor);
    out.origin_y=real(0x63c50);out.point_distance=real(0x63b9c);
    // Eleven original per-player firing-geometry transforms independently
    // establish focus400, center249/192 and the aim-point Y-offset convention.
    out.screen_inverse_valid=true;
    return out;
}
struct NativeAimPoint {double x{},y{},z{};bool valid{};};
inline NativeAimPoint native_aim_point(const NativeAimCalibration& c,uint8_t x,uint8_t y) {
    if(!c.valid||x<c.left||x>c.right||y<c.bottom||y>c.top||
       c.left>=c.right||c.bottom>=c.top||!std::isfinite(c.yaw_scale)||c.yaw_scale<=0||
       !std::isfinite(c.point_distance)||c.point_distance<=0)return {};
    for(float value:{c.angle_units_x,c.angle_units_y,c.previous_yaw,c.minimum_pitch,c.origin_x,c.origin_y})
        if(!std::isfinite(value))return {};
    if(c.angle_units_x<=0||c.angle_units_x>32767||c.angle_units_y<=0||c.angle_units_y>32767||
       c.minimum_pitch>0||c.minimum_pitch<-32768||std::abs(c.previous_yaw)>c.angle_units_x)return {};
    constexpr double angle=6.283185307179586476925286766559/65536.;
    // Preserve binary32 operations and truncation at the original i960 stages.
    const float nx=(float(x)-c.left)/((c.right-c.left)*.5f)-1.f;
    const int yaw=int(-nx*c.angle_units_x);
    const int prior=int(c.previous_yaw/c.yaw_scale);
    const float correction=float(std::cos(prior*angle)*768.);
    const int upper=int(correction+c.angle_units_y);
    if(upper<int(c.minimum_pitch))return {};
    const float ny=(float(y)-c.bottom)/((c.top-c.bottom)*.5f)-1.f;
    const int pitch=std::clamp(int(ny*upper),int(c.minimum_pitch),upper);
    const double a=yaw*angle,b=pitch*angle,d=c.point_distance;
    return {-std::sin(a)*std::cos(b)*d+c.origin_x,std::sin(b)*d+c.origin_y,
        std::cos(a)*std::cos(b)*d,true};
}
inline AimValue native_screen_to_adc(unsigned player,double u,double v,const NativeAimCalibration& c) {
    if(player>1||!c.screen_inverse_valid||!std::isfinite(u)||!std::isfinite(v)||
       u<0||u>1||v<0||v>1||!native_aim_point(c,c.center_x,c.center_y).valid)return {};
    constexpr double angle=6.283185307179586476925286766559/65536.;
    // Match NativeLens::project_point's inclusive pixel-center contract.
    const double px=u*495.,py=v*383.;
    const double yaw=-std::atan((px-249.)/400.);
    const double pitch=std::atan((192.-py)/400.*std::cos(yaw));
    const int prior=int(c.previous_yaw/c.yaw_scale);
    const float correction=float(std::cos(prior*angle)*768.);
    const int upper=int(correction+c.angle_units_y);
    if(upper<=0)return {};
    const int x0=int(std::lround(c.left+(c.right-c.left)*.5*(1.-yaw/angle/c.angle_units_x)));
    const int y0=int(std::lround(c.bottom+(c.top-c.bottom)*.5*(1.+pitch/angle/upper)));
    // Choose the closest original eight-bit native result, including its
    // integer truncation and asymmetric pitch clamp. Unreachable HUD/border
    // targets cannot authorize a shot by silently pinning to a cabinet stop.
    AimValue out;double best=16.; // At most four native pixels of quantization.
    for(int x=std::max(int(c.left),x0-2);x<=std::min(int(c.right),x0+2);++x)
        for(int y=std::max(int(c.bottom),y0-2);y<=std::min(int(c.top),y0+2);++y) {
            const auto point=native_aim_point(c,uint8_t(x),uint8_t(y));
            if(!point.valid||point.z<=0)continue;
            const double dx=249.+(point.x-c.origin_x)/point.z*400.-px;
            const double dy=192.-(point.y-c.origin_y)/point.z*400.-py;
            const double error=dx*dx+dy*dy;
            if(error<best){best=error;out={uint8_t(x),uint8_t(y),true};}
        }
    return out;
}
}
