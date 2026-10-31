// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include "game_state.h"
namespace bel {
// Original BEL Service/Gun Calibration protocol, observed with cabinet inputs.
// Initialize only the entirely erased eight-byte gun configuration. The game
// performs every setting/store/checksum/save/reboot; this policy writes no RAM.
class GunSetupPolicy {
    bool active_{},completed_{},failed_{},stops_done_{},saving_{};
    unsigned ticks_{},down_{},gap_{};
    Cabinet held_;
public:
    struct Step {bool active{};Cabinet cabinet;};
    bool completed()const{return completed_;}
    bool failed()const{return failed_;}
    void release(){down_=0;gap_=24;held_={};}
    Step update(const GameState& s,bool allowed) {
        if(completed_||failed_)return {};
        if(!active_) {
            if(!allowed||!s.valid||!s.ready||s.service||!s.calibration_erased||
                s.players[0].joined()||s.players[1].joined())return {};
            active_=true;
        }
        if(!allowed){release();return {true,{}};}
        if(++ticks_>57*45){failed_=true;active_=false;return {};}
        if(saving_&&s.valid&&!s.service&&s.calibration_config_valid) {
            completed_=true;active_=false;return {};
        }
        if(down_){--down_;return {true,held_};}
        if(gap_){--gap_;return {true,{}};}
        Cabinet next;
        if(!s.valid)return {true,{}}; // Original save/reboot invalidates RAM briefly.
        if(!s.service) {
            if(!saving_)next.test=true;
        } else if(s.gun_calibration) {
            const auto sub=s.service_submode;
            if(sub==3||sub==5||sub==7||sub==9) {
                const unsigned p=sub>=7?1:0;const bool upper=sub==5||sub==9;
                next.x[p]=upper?NativeRanges[p].right:NativeRanges[p].left;
                next.y[p]=upper?NativeRanges[p].top:NativeRanges[p].bottom;
                next.trigger[p]=true;
            } else if(sub==11){stops_done_=true;next.test=true;}
        } else if(s.service_menu==0&&s.service_selection<=10) {
            if(!stops_done_) {
                if(s.service_selection==3)next.test=true;
                else next.service=true;
            } else if(s.service_selection==10){next.test=true;saving_=true;}
            else next.service=true;
        }
        if(next.test||next.service||next.trigger[0]||next.trigger[1]) {
            held_=next;down_=11;gap_=24;return {true,next};
        }
        return {true,{}};
    }
};
}
