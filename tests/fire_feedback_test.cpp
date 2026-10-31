#include "../native/game_state.h"
#include "../native/input_policy.h"
#include <cassert>
#include <map>
using namespace bel;
int main() {
    // Original BEL fire gate, projectile allocation/owner and shot-counter
    // instructions, independently checked against native projectile traces.
    std::map<uint32_t,uint32_t> memory{{0x60bc8,0x350360bc},{0x60c00,0x09fb7c80},
        {0x60c78,0x90b8e024},{0x60c7c,0x59bdc801},{0x60c80,0x92b8e024},
        {0x188d4,0x8240e030},{0x5b4964,100},{0x5b49e8,200}};
    auto read=[&](uint32_t a){return memory[a];};
    GameState before;before.valid=before.ready=before.playing=true;before.session=7;
    for(auto& p:before.players){p.state=1;p.health=1;}
    ShotTracker tracker;tracker.update(before,read);
    assert(before.counting_shots&&before.shot_counters_valid);
    assert(before.players[0].shots==100&&before.players[1].shots==200);
    CabinetInput delivered;delivered.hand={1,0};delivered.cabinet.trigger[0]=true;
    ShotOwnership owner;owner.update(before,before,delivered);
    Hands hands;for(auto& h:hands){h.tracked=true;h.aim={.5,.5,true,true};}
    Settings settings;settings.p1_hand=1;InputPolicy policy;
    policy.update(0,hands,before,settings,true,false,&owner);
    memory[0x5b4964]++;memory[0x5b49e8]++;
    auto after=before;tracker.update(after,read);owner.update(after,before,delivered);
    auto feedback=policy.update(.1,hands,after,settings,true,false,&owner);
    assert(feedback.accepted_shots[0]==1&&feedback.hand[0]==1);
    assert(!feedback.accepted_shots[1]); // No delivered P2 action: no attribution.
    assert(!policy.update(.2,hands,after,settings,true,false,&owner).accepted_shots[0]);
    // A counter reset, foreign program, service or invalid state cannot fabricate feedback.
    memory[0x5b4964]=0;auto reset=after;tracker.update(reset,read);owner.update(reset,after,delivered);
    assert(owner.shots_by_hand[0][1]==1);
    memory[0x60c80]=0;tracker.update(after,read);assert(!after.shot_counters_valid);
    memory[0x60c80]=0x92b8e024;after.service=true;tracker.update(after,read);assert(!after.counting_shots);
    after.service=false;after.valid=false;tracker.update(after,read);assert(!after.counting_shots);
}
