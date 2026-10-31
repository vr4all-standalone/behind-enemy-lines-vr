// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <cmath>
#include "input_policy.h"
#include "game_state.h"
namespace bel {
// Match Teraburst's short visual pulse. Events still come exclusively from
// BEL's verified native firing acknowledgement; the native fire rate is intact.
inline float automatic_recoil(double age){return std::isfinite(age)&&age>=0&&age<.08?float(std::sin(age/.08*3.141592653589793)*std::exp(-age*30)):0;}
inline constexpr double BurstPulseInterval=(656.*424./16000000.)*16./3.;
class BurstFeedback {
    struct Burst {int hand{-1};unsigned remaining{};double next{};};
    std::array<Burst,2> bursts_{};uint64_t session_{};uint8_t joined_{};double previous_{-1};
public:
    void suspend(){bursts_={};previous_=-1;session_=0;joined_=0;}
    CabinetInput update(double now,const CabinetInput& input,const GameState& state,const Hands& hands,bool enabled){
        auto out=input;out.accepted_shots={};
        if(!std::isfinite(now)){suspend();return out;}
        if(now<previous_||session_!=input.native_session||joined_!=input.native_joined)bursts_={};
        previous_=now;session_=input.native_session;joined_=input.native_joined;
        for(unsigned p=0;p<2;p++){
            auto& burst=bursts_[p];const int hand=input.hand[p];
            const bool valid=enabled&&state.valid&&state.playing&&!state.service&&state.session==input.native_session&&
                state.players[p].alive()&&(input.native_joined&(1<<p))&&hand>=0&&hand<2&&hands[hand].tracked;
            if(!valid){burst={};continue;}
            if(burst.hand!=hand)burst={};
            // The original acknowledgement represents the game's audible
            // three-round burst. Never synthesize a native projectile or alter
            // its cadence; schedule only local feedback, owned by that hand.
            if(input.accepted_shots[p])burst={hand,3,now};
            if(!burst.remaining||now<burst.next)continue;
            if(now-burst.next>=.06){burst={};continue;} // No replay after a stall.
            out.accepted_shots[p]=1;--burst.remaining;burst.next+=BurstPulseInterval;
        }
        return out;
    }
};
}
