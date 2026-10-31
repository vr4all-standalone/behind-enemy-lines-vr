// SPDX-License-Identifier: GPL-2.0-or-later
#include "../native/input_policy.h"
#include <cassert>
using namespace bel;
static Hands tracked() {
    Hands h;for(auto& hand:h){hand.tracked=true;hand.aim={.5,.5,true,true};}return h;
}
int main() {
    Settings settings;
    NativeState state;state.valid=state.start_ready=state.fire_ready=true;state.joined_mask=1;state.session=1;
    Hands hands=tracked();InputPolicy policy;
    policy.update(0,hands,state,settings,true,false);
    hands[0].pump=true;
    auto output=policy.update(.1,hands,state,settings,true,false);
    assert(in1(0xff,output.cabinet)==0xef); // Native P1 Missile, no invented reload.
    output=policy.update(.2,hands,state,settings,true,false);
    assert(in1(0xff,output.cabinet)==0xef); // Preserve native held-button behavior.
    policy.update(.3,hands,state,settings,false,false);
    output=policy.update(.4,hands,state,settings,true,false);
    assert(in1(0xff,output.cabinet)==0xff); // Held across focus cannot launch.
    hands[0].pump=false;policy.update(.5,hands,state,settings,true,false);
    hands[0].pump=true;output=policy.update(.6,hands,state,settings,true,false);
    assert(in1(0xff,output.cabinet)==0xef);
    policy.update(.7,hands,state,settings,true,true);
    output=policy.update(.8,hands,state,settings,true,false);
    assert(in1(0xff,output.cabinet)==0xff);
    // Reversible dual ownership: held old button cannot become another player.
    state.joined_mask=3;settings.p1_hand=1;
    hands[1].pump=true;output=policy.update(.9,hands,state,settings,true,false);
    assert(in1(0xff,output.cabinet)==0xff);
    hands[0].pump=hands[1].pump=false;policy.update(1,hands,state,settings,true,false);
    hands[0].pump=true;output=policy.update(1.1,hands,state,settings,true,false);
    assert(in1(0xff,output.cabinet)==0xdf); // P2 now owns left hand.
    hands[0].tracked=false;policy.update(1.2,hands,state,settings,true,false);
    hands[0].tracked=true;output=policy.update(1.3,hands,state,settings,true,false);
    assert(in1(0xff,output.cabinet)==0xff);
    // Unknown native state cannot fabricate joined players, shots or controls.
    state.valid=false;output=policy.update(1.4,hands,state,settings,true,false);
    assert(in1(0xff,output.cabinet)==0xff && output.cabinet.starts==0);
    assert(output.accepted_shots[0]==0&&output.accepted_shots[1]==0);
    // Either solo hand starts native P1. A held Start cannot become P2 when
    // authoritative game state acknowledges P1; only a fresh other-hand press.
    {
        InputPolicy starts;auto h=tracked();NativeState native;
        native.valid=native.start_ready=native.fire_ready=true;native.session=7;
        starts.update(0,h,native,settings,true,false);
        h[0].start=true;auto c=starts.update(.01,h,native,settings,true,false);
        assert(c.cabinet.coins==1&&c.cabinet.starts==0);
        c=starts.update(.16,h,native,settings,true,false);assert(c.cabinet.starts==1);
        native.joined_mask=1;c=starts.update(.17,h,native,settings,true,false);
        assert(c.cabinet.starts==0&&c.cabinet.coins==0);
        c=starts.update(.18,h,native,settings,true,false);assert(c.cabinet.starts==0);
        h[0].start=false;starts.update(.19,h,native,settings,true,false);
        h[0].start=true;c=starts.update(.2,h,native,settings,true,false);
        assert(c.cabinet.coins==2); // Layout was reversed earlier: left is P2.
    }
    // A main-gun pull that begins outside cannot become a shot on re-entry.
    {
        InputPolicy fire;auto h=tracked();NativeState native;
        native.valid=native.start_ready=native.fire_ready=true;native.joined_mask=3;native.session=8;
        fire.update(0,h,native,settings,true,false);
        h[1].trigger=true;h[1].aim.inside=false;
        auto c=fire.update(.1,h,native,settings,true,false);assert(!c.cabinet.trigger[0]);
        h[1].aim.inside=true;c=fire.update(.2,h,native,settings,true,false);assert(!c.cabinet.trigger[0]);
        h[1].trigger=false;fire.update(.3,h,native,settings,true,false);
        h[1].trigger=true;c=fire.update(.4,h,native,settings,true,false);assert(c.cabinet.trigger[0]);
    }
    // Native P2 can remain/continue independently after P1 has left. Neither
    // hand layout nor missing P1 tracking may silently redirect or drop Start.
    for(int layout=0;layout<2;layout++) {
        Settings s;s.p1_hand=layout;Hands h;const int p2hand=1-layout;
        h[p2hand].tracked=true;h[p2hand].aim={.5,.5,true,true};
        NativeState native;native.valid=native.start_ready=native.credits_valid=true;
        native.joined_mask=native.continue_mask=2;native.continue_cost=1;native.session=20+layout;
        InputPolicy p;p.update(0,h,native,s,true,false);
        h[p2hand].start=true;auto c=p.update(.01,h,native,s,true,false);
        assert(c.cabinet.coins==2&&c.cabinet.starts==0);
        h[p2hand].start=false;native.credits=1;
        c=p.update(.26,h,native,s,true,false);
        assert(c.cabinet.starts==2&&c.cabinet.coins==0);
    }
    for(int layout=0;layout<2;layout++)for(int player=0;player<2;player++) {
        Settings s;s.p1_hand=layout;Hands h;
        const int transferred=player==0?1-layout:layout;
        h[transferred].tracked=true;h[transferred].aim={.5,.5,true,true};
        NativeState native;native.valid=native.start_ready=native.credits_valid=true;
        native.joined_mask=native.continue_mask=1<<player;native.continue_cost=1;
        InputPolicy p;p.update(0,h,native,s,true,false);h[transferred].start=true;
        const auto c=p.update(.01,h,native,s,true,false);
        assert(c.cabinet.coins==(1<<player)); // Solo hand transfer preserves the native seat.
    }
}
