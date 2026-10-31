#include "../native/automatic_feedback.h"
#include <cassert>
using namespace bel;
int main(){
    GameState state;state.valid=state.playing=true;state.session=7;
    for(auto& p:state.players){p.state=1;p.health=1;}
    CabinetInput input;input.hand={1,0};input.native_joined=3;input.native_session=7;
    Hands hands;for(auto& h:hands)h.tracked=true;
    BurstFeedback bursts;input.accepted_shots={1,0};
    auto first=bursts.update(1,input,state,hands,true);assert(first.accepted_shots[0]==1&&!first.accepted_shots[1]);
    input.accepted_shots={};
    assert(!bursts.update(1.04,input,state,hands,true).accepted_shots[0]);
    assert(bursts.update(1+BurstPulseInterval+.001,input,state,hands,true).accepted_shots[0]==1);
    assert(bursts.update(1+2*BurstPulseInterval+.001,input,state,hands,true).accepted_shots[0]==1);
    assert(!bursts.update(1.3,input,state,hands,true).accepted_shots[0]);
    assert(!bursts.update(1.5,input,state,hands,true).accepted_shots[0]);
    // Tracking/focus/menu/death, session changes, player/hand transitions and
    // long stalls must cancel queued sub-pulses instead of replaying them.
    for(int reason=0;reason<7;reason++){
        bursts.suspend();input.hand={1,0};input.native_session=state.session=7;input.native_joined=3;
        state.players[0].health=1;hands[1].tracked=true;input.accepted_shots={1,1};
        assert(bursts.update(2,input,state,hands,true).accepted_shots[0]==1);input.accepted_shots={};
        bool enabled=true;
        if(reason==0)enabled=false;
        if(reason==1)hands[1].tracked=false;
        if(reason==2)state.players[0].health=0;
        if(reason==3)input.native_session=state.session=8;
        if(reason==4)input.hand={0,1};
        if(reason==5)input.native_joined=2;
        double now=reason==6?2.4:2.02;
        assert(!bursts.update(now,input,state,hands,enabled).accepted_shots[0]);
        hands[1].tracked=true;state.players[0].health=1;
        assert(!bursts.update(now+.01,input,state,hands,true).accepted_shots[0]);
    }
    // An acknowledged native burst completes after trigger release; raw
    // physical presses without an acknowledgement never schedule feedback.
    bursts.suspend();input.hand={1,0};input.native_session=state.session=9;input.native_joined=3;
    input.accepted_shots={0,1};auto p2=bursts.update(3,input,state,hands,true);assert(p2.accepted_shots[1]==1&&p2.hand[1]==0);
    input.accepted_shots={};hands[0].trigger=false;
    assert(bursts.update(3+BurstPulseInterval+.001,input,state,hands,true).accepted_shots[1]==1);
    bursts.suspend();hands[0].trigger=true;assert(!bursts.update(4,input,state,hands,true).accepted_shots[1]);
}
