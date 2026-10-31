// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include "cabinet_io.h"
#include "reticle_mode.h"
namespace bel {
struct Aim {double u{.5},v{.5};bool valid{},inside{};};
struct HandInput {bool tracked{},trigger{},start{},pump{};Aim aim;};
using Hands=std::array<HandInput,2>; // Left, right.
inline int start_player_for_hand(int hand,int p1_hand,uint8_t joined,const Hands& hands) {
    joined&=3;
    if(!joined)return 0;
    if(joined!=3&&hands[0].tracked!=hands[1].tracked)return joined==2?1:0;
    return hand==(p1_hand==0?0:1)?0:1;
}
enum class BackgroundMode {Original,Depth3D};
inline constexpr std::array<float,4> HapticLevels{0,.35f,.65f,1};
inline int haptic_level(float value){int best=0;for(int i=1;i<4;i++)if(std::abs(value-HapticLevels[i])<std::abs(value-HapticLevels[best]))best=i;return best;}
struct Settings {
    int p1_hand{0};std::array<Calibration,2> calibration=NativeRanges;
    float world_scale{.1f},wide_world_scale{.830f},panel_distance{1.8f},gun_angle{},gun_scale{1},haptics{.65f},view_fov{65};
    float reticle_min_size{.35f},reticle_max_size{1.45f},reticle_custom_min_size{.30f},reticle_custom_max_size{1.f},reticle_shrink_distance{10.f};ReticleMode reticle_mode{ReticleMode::Ingame};
    float hud_size{1.2f};
    bool laser{true},wider_view{true};int gun{};
    BackgroundMode background{BackgroundMode::Original};
};
inline float& active_world_scale(Settings& s){return s.wider_view?s.wide_world_scale:s.world_scale;}
inline float active_world_scale(const Settings& s){return s.wider_view?s.wide_world_scale:s.world_scale;}
// Populate only from separately verified native observations. In particular,
// physical trigger state must never stand in for an accepted native shot.
struct NativeState {
    bool valid{},start_ready{},service{},shot_counters_valid{};
    bool fire_ready{},credits_valid{},free_play{};
    uint32_t credits{},start_cost{1},continue_cost{1};
    uint8_t continue_mask{};
    bool shot_ownership_valid{};
    uint8_t joined_mask{};
    uint64_t session{};
    std::array<uint32_t,2> shots{};
    std::array<std::array<uint32_t,2>,2> shots_by_hand{};
};
struct CabinetInput {
    Cabinet cabinet;
    std::array<int,2> hand{-1,-1};
    std::array<uint32_t,2> accepted_shots{};
    Hands aim_hands{};
    uint64_t native_session{};
    uint8_t native_joined{};
    bool service_guns{};
};
struct ShotOwnership {
    bool valid{};uint64_t session{};
    std::array<std::array<uint32_t,2>,2> shots_by_hand{};
    template<class State> void update(const State& current,const State& previous,const CabinetInput& delivered) {
        if(session!=current.session){*this={};session=current.session;}
        valid=current.valid&&current.counting_shots&&current.shot_counters_valid;
        if(!valid || !previous.valid || !previous.counting_shots || !previous.shot_counters_valid ||
           previous.session!=current.session)return;
        for(unsigned player=0;player<2;player++) {
            const auto before=previous.players[player].shots,after=current.players[player].shots;
            const int hand=delivered.hand[player];
            // Only observed accepted projectiles with a known delivered owner
            // can later acknowledge a hand. Unknown ownership is discarded.
            if(hand>=0 && hand<2 && delivered.cabinet.trigger[player] && after>=before && after-before<=2)
                shots_by_hand[player][hand]+=after-before;
        }
    }
};
class CabinetLatch {
    CabinetInput previous_;
public:
    CabinetInput update(const CabinetInput& desired) {
        auto out=desired;
        for(int p=0;p<2;p++)if(previous_.hand[p]!=desired.hand[p]) {
            if(previous_.cabinet.trigger[p]&&desired.cabinet.trigger[p])out.cabinet.trigger[p]=false;
            if(previous_.cabinet.missile[p]&&desired.cabinet.missile[p])out.cabinet.missile[p]=false;
        }
        previous_=out;return out;
    }
};
class OperatorLatch {
    std::array<unsigned,4> ticks_{};
public:
    uint16_t update(uint16_t pending) {
        uint16_t out{};constexpr unsigned buttons[]={4,8,0x10,0x20};
        for(int i=0;i<4;i++){if(pending&buttons[i])ticks_[i]=12;if(ticks_[i]){out|=buttons[i];--ticks_[i];}}
        return out;
    }
};
class InputPolicy {
    Hands previous_{};
    NativeState previous_state_{};
    std::array<bool,2> blocked_{true,true},outside_pull_{},start_waiting_{true,true};
    std::array<bool,2> missile_blocked_{true,true},missile_outside_pull_{};
    struct StartPulse {double time{};int player{-1};double start_time{-1};bool native_credits{},coin_required{};};
    std::array<StartPulse,2> pulses_{};
    std::array<int,2> start_owner_{-1,-1};
    bool initialized_{},was_enabled_{},was_ready_{};
    uint8_t previous_joined_{};
    int previous_layout_{0},solo_hand_{0};
    double previous_time_{};
public:
    void suspend(){*this=InputPolicy{};}
    template<class State> CabinetInput update(double now,const Hands& hands,const State& state,
                        const Settings& settings,bool focused,bool menu,const ShotOwnership* ownership=nullptr) {
        NativeState native;native.valid=state.valid;native.start_ready=state.ready;
        native.fire_ready=state.playing;native.credits_valid=state.credits_valid;
        native.credits=state.credits;native.start_cost=state.start_cost;native.continue_cost=state.continue_cost;native.free_play=state.free_play;
        native.service=state.service;native.session=state.session;
        native.shot_counters_valid=state.counting_shots&&state.shot_counters_valid;
        if(ownership && ownership->valid && ownership->session==state.session) {
            native.shot_ownership_valid=true;native.shots_by_hand=ownership->shots_by_hand;
        }
        for(int p=0;p<2;p++){if(state.players[p].joined()){native.joined_mask|=1<<p;if(!state.players[p].alive())native.continue_mask|=1<<p;}native.shots[p]=state.players[p].shots;}
        return update(now,hands,native,settings,focused,menu);
    }
    CabinetInput update(double now,const Hands& hands,const NativeState& state,
                        const Settings& settings,bool focused,bool menu) {
        CabinetInput out;
        if(!std::isfinite(now)){suspend();return out;}
        if(initialized_&&now<previous_time_)suspend();
        const int layout=settings.p1_hand==0?0:1;
        const uint8_t joined=state.valid?state.joined_mask&3:0;
        out.aim_hands=hands;out.native_session=state.session;out.native_joined=joined;
        const bool enabled=focused&&!menu&&state.valid&&!state.service;
        const bool fire_enabled=enabled&&state.fire_ready;
        const bool ready=enabled&&state.start_ready;
        const bool session_changed=initialized_&&state.session!=previous_state_.session;
        const bool transition=!initialized_||enabled!=was_enabled_||joined!=previous_joined_||layout!=previous_layout_||session_changed||state.fire_ready!=previous_state_.fire_ready;
        if(transition){blocked_={true,true};outside_pull_={};missile_blocked_={true,true};missile_outside_pull_={};}
        if(!initialized_||layout!=previous_layout_)solo_hand_=layout;
        if(joined==3)solo_hand_=layout;
        else if(previous_joined_==3 && joined==2)solo_hand_=1-layout;
        const int previous_solo=solo_hand_;
        if(joined!=3&&enabled) {
            const int other=1-solo_hand_;
            if(!hands[solo_hand_].trigger&&hands[other].tracked&&
               (!hands[solo_hand_].tracked || (hands[other].trigger&&!previous_[other].trigger&&!transition)))solo_hand_=other;
        }
        if(solo_hand_!=previous_solo&&previous_[solo_hand_].trigger)blocked_[solo_hand_]=true;
        if(solo_hand_!=previous_solo)missile_blocked_={true,true};
        if(joined==3)out.hand={layout,1-layout};
        else if(joined==1)out.hand={solo_hand_,-1};
        else if(joined==2)out.hand={-1,solo_hand_};
        for(int h=0;h<2;h++) {
            const auto& in=hands[h];
            const bool aim_ok=in.aim.valid&&std::isfinite(in.aim.u)&&std::isfinite(in.aim.v);
            if(!fire_enabled||!in.tracked||!aim_ok)blocked_[h]=true;
            else if(!in.trigger){blocked_[h]=false;outside_pull_[h]=false;}
            if(in.trigger&&!previous_[h].trigger)outside_pull_[h]=!in.aim.inside;
            if(!fire_enabled||!in.tracked||!aim_ok)missile_blocked_[h]=true;
            else if(!in.pump){missile_blocked_[h]=false;missile_outside_pull_[h]=false;}
            if(in.pump&&!previous_[h].pump)missile_outside_pull_[h]=!in.aim.inside;
            if(!ready||!in.tracked||ready!=was_ready_||layout!=previous_layout_||session_changed) {
                start_waiting_[h]=true;pulses_[h]={};start_owner_[h]=-1;
            }
            if(joined!=previous_joined_) {
                pulses_[h]={};start_owner_[h]=-1;
                // A held solo Start must not become P2 Start when the cabinet
                // acknowledges P1. A genuinely fresh press may still join P2.
                if(in.start&&previous_[h].start)start_waiting_[h]=true;
            }
            if(ready&&in.tracked&&!in.start)start_waiting_[h]=false;
            if(!in.start)start_owner_[h]=-1;
            if(ready&&in.tracked&&!start_waiting_[h]&&in.start&&!previous_[h].start) {
                const int player=start_player_for_hand(h,layout,joined,hands);
                if(player==0 || (joined&(1<<player)) || ((joined&1)&&hands[0].tracked&&hands[1].tracked)) {
                    start_owner_[h]=player;
                    auto& pulse=pulses_[h];
                    if(pulse.player!=player || now-pulse.time>=.26) {
                        pulse={now,player};pulse.native_credits=state.credits_valid;
                        const auto cost=state.continue_mask&(1<<player)?state.continue_cost:state.start_cost;
                        pulse.coin_required=!state.free_play&&state.credits<cost;
                    }
                }
            }
        }
        for(int p=0;p<2;p++) {
            const int h=out.hand[p];if(h<0)continue;
            const auto& hand=hands[h];
            const auto aim=map_aim(p,hand.aim.u,hand.aim.v,settings.calibration[p]);
            if(hand.tracked&&hand.aim.valid&&aim.valid) {
                out.cabinet.x[p]=aim.x;out.cabinet.y[p]=aim.y;
                out.cabinet.trigger[p]=fire_enabled&&!blocked_[h]&&hand.trigger&&hand.aim.inside&&!outside_pull_[h];
                out.cabinet.missile[p]=fire_enabled&&!missile_blocked_[h]&&hand.pump&&hand.aim.inside&&!missile_outside_pull_[h];
            }
        }
        if(ready)for(int h=0;h<2;h++) {
            auto& pulse=pulses_[h];if(pulse.player<0)continue;
            const double elapsed=now-pulse.time;
            if(pulse.native_credits) {
                const auto cost=state.continue_mask&(1<<pulse.player)?state.continue_cost:state.start_cost;
                const bool credited=state.credits_valid&&(state.free_play||state.credits>=cost);
                if(elapsed>=0&&elapsed<.1&&pulse.coin_required&&!credited)out.cabinet.coins|=1<<pulse.player;
                // Native BEL credits arrive after coin release/debounce. Keep a
                // tapped request pending, then issue a full native Start pulse.
                if(pulse.start_time<0&&elapsed>=0&&elapsed<.75&&credited&&(!pulse.coin_required||elapsed>=.14))pulse.start_time=now;
                if(pulse.start_time>=0&&(now-pulse.start_time<.12||(hands[h].start&&start_owner_[h]==pulse.player)))out.cabinet.starts|=1<<pulse.player;
            } else {
                if(elapsed>=0&&elapsed<.1)out.cabinet.coins|=1<<pulse.player;
                if(elapsed>=.14&&(elapsed<.26||(hands[h].start&&start_owner_[h]==pulse.player)))out.cabinet.starts|=1<<pulse.player;
            }
        }
        if(enabled&&!transition&&state.shot_counters_valid&&state.shot_ownership_valid&&previous_state_.valid&&
           previous_state_.shot_counters_valid&&previous_state_.shot_ownership_valid&&state.session==previous_state_.session) {
            for(int p=0;p<2;p++)if(joined&(1<<p)) {
                const int hand=out.hand[p];if(hand<0 || hand>1)continue;
                const auto before=previous_state_.shots_by_hand[p][hand],current=state.shots_by_hand[p][hand];
                if(current>=before&&current-before<=64)out.accepted_shots[p]=current-before;
            }
        }
        previous_=hands;previous_state_=state;previous_layout_=layout;previous_joined_=joined;
        was_enabled_=enabled;was_ready_=ready;previous_time_=now;initialized_=true;
        return out;
    }
};
}
