// SPDX-License-Identifier: GPL-2.0-or-later
#include "../native/game_state.h"
#include "../native/input_policy.h"
#include <cassert>
#include <map>
int main() {
    // A foreign game's known fingerprint must never authorize BEL gameplay.
    std::map<uint32_t,uint32_t> foreign{{0x075d64,0x8c203000},{0x075d68,0x00532d54},
        {0x0ed8f8,0x58a50980},{0x06d414,0x90a16014},
        {0x0f0bb0,0x0054bdc4},{0x0f1bcc,0x000f0ba0}};
    auto read=[&](uint32_t address){return foreign[address];};
    const auto state=bel::read_game_state(read);
    assert(!state.valid && !state.ready && !state.playing);
    assert(!state.counting_shots && !state.shot_counters_valid);
    assert(!state.players[0].joined()&&!state.players[1].joined());
    assert(!bel::read_native_calibration(read,0).valid);
    bel::GameState unknown;unknown.session=10;
    bel::ShotTracker tracker;tracker.update(unknown,read);
    assert(!unknown.shot_counters_valid && unknown.players[0].shots==0);
    // Literal BEL instruction/address words from the verified original CPU
    // payload and native trace: title -> P1 Start -> P2 join -> native Service.
    std::map<uint32_t,uint32_t> memory{
        {0x56690,0x80803000},{0x56694,0x005fa12c},
        {0x566ac,0x90903903},{0x566b0,0x029767f0},
        {0x513e0,0x8ca03000},{0x513e4,0x01c00002},
        {0x5d150,0xca91a05a},{0x5d154,0xca91a056},
        {0x6957c,0x82883000},{0x69580,0x005a8831},
        {0x12a4,0x82b03000},{0x12a8,0x005a8831},
        {0x5a8830,0x00000102},{0x5fa12c,4},
        {0x5a85e8,0x005a85f0},{0x5a85fc,0x01000000},{0x5a8600,1}
    };
    auto native_read=[&](uint32_t address){return memory[address];};
    auto title=bel::read_game_state(native_read);
    assert(title.valid&&title.ready&&!title.playing&&!title.service);
    memory[0x5b4b5c]=4;
    assert(!bel::read_game_state(native_read).ready); // Initial logo pass ignores native Start.
    memory[0x5b4b5c]=0;
    assert(!title.players[0].joined()&&!title.players[1].joined());
    bel::Hands hands;hands[0].tracked=true;hands[0].aim={.5,.5,true,true};
    bel::InputPolicy start;bel::Settings settings;
    start.update(0,hands,title,settings,true,false);
    hands[0].start=true;
    assert(start.update(.01,hands,title,settings,true,false).cabinet.coins==1);
    hands[0].start=false;
    assert(start.update(.16,hands,title,settings,true,false).cabinet.starts==0);
    // BEL counts a coin after release/debounce (~0.25s in native traces).
    // A quick A/X tap must wait for that real credit acknowledgement.
    memory[0x5a87f0]=1;title=bel::read_game_state(native_read);
    assert(start.update(.26,hands,title,settings,true,false).cabinet.starts==1);
    memory[0x5fa12c]=11;memory[0x5b4994]=0x0a8c0000;
    auto p1=bel::read_game_state(native_read);
    assert(p1.valid&&p1.players[0].joined()&&!p1.players[1].joined());
    assert(!p1.playing&&!p1.ready); // Native introduction is not world gameplay.
    memory[0x5fa12c]=21;memory[0x5b4a18]=0x0a8c0000;
    auto dual=bel::read_game_state(native_read);
    assert(dual.playing&&dual.ready&&dual.players[0].alive()&&dual.players[1].alive());
    assert(!dual.shot_counters_valid); // Native activation is not a shot event.
    // Actual death/Continue modes from Quest logs and original native Start
    // handlers must retain coin/Start without authorizing gameplay firing.
    memory[0x59a88]=0x90903000;memory[0x59a8c]=0x005a0cfc;
    memory[0x5a310]=0x90803000;memory[0x5a314]=0x005a0cfc;
    memory[0x596c8]=0x0900d858;
    for(unsigned mode:{23u,27u,29u}){
        memory[0x5fa12c]=mode;memory[0x5b4994]=0xffff0000;
        auto dead=bel::read_game_state(native_read);
        assert(dead.ready&&!dead.playing&&dead.players[0].joined()&&!dead.players[0].alive());
        memory[0x5b4994]=0x0a8c0000;
        assert(!bel::read_game_state(native_read).ready);
    }
    memory[0x5b4994]=0xffff0000;memory[0x5fa12c]=28;
    assert(!bel::read_game_state(native_read).ready);
    memory[0x5fa12c]=29;memory[0x596c8]=0;
    assert(!bel::read_game_state(native_read).ready);
    memory[0x596c8]=0x0900d858;memory[0x5b4994]=0x0a8c0000;
    memory[0x5fa12c]=21;
    memory[0x5a8830]=0x0000ff02;memory[0x5a883c]=0x00000303;memory[0x5b4b64]=3;
    auto service=bel::read_game_state(native_read);
    assert(service.valid&&service.service&&!service.ready&&!service.playing);
    assert(service.gun_calibration&&service.service_busy==1);
    memory[0x5b4b64]=7;
    assert(bel::read_game_state(native_read).service_busy==2);
    memory[0x5b4b64]=9;
    assert(bel::read_game_state(native_read).service_busy==2);
    memory[0x5b4b64]=11;
    assert(!bel::read_game_state(native_read).service_busy);
    // Exactly the original erased eight-byte gun configuration authorizes
    // first-run setup. Partial/existing calibration must remain untouched.
    memory[0x5a8604]=0xff000001;memory[0x5a8608]=memory[0x5a860c]=0xffffffff;
    auto erased=bel::read_game_state(native_read);
    assert(erased.calibration_erased&&!erased.calibration_config_valid);
    memory[0x5a8604]=0x69000001;memory[0x5a8608]=0x96111100;memory[0x5a860c]=0x009d9d96;
    auto saved=bel::read_game_state(native_read);
    assert(!saved.calibration_erased&&saved.calibration_config_valid);
    memory[0x56694]=0;assert(!bel::read_game_state(native_read).valid);
    {
        bel::GameState intro;intro.valid=true;intro.players[0].state=1;intro.players[0].health=1;
        bel::InputPolicy fire;bel::Hands h;h[0].tracked=true;h[0].aim={.5,.5,true,true};
        fire.update(0,h,intro,settings,true,false);
        h[0].trigger=true;assert(!fire.update(.1,h,intro,settings,true,false).cabinet.trigger[0]);
        intro.playing=true;assert(!fire.update(.2,h,intro,settings,true,false).cabinet.trigger[0]);
        h[0].trigger=false;fire.update(.3,h,intro,settings,true,false);
        h[0].trigger=true;assert(fire.update(.4,h,intro,settings,true,false).cabinet.trigger[0]);
    }
}
