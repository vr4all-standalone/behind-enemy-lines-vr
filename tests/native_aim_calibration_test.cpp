// SPDX-License-Identifier: GPL-2.0-or-later
#include "../native/aim_policy.h"
#include "../native/shared_runtime.h"
#include "../native/projection.h"
#include "../native/controller_controls.h"
#include <bit>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

struct NativeFixture {
    // Independently recorded original instructions and calibrated live RAM.
    std::map<uint32_t,uint32_t> words{
        {0x56690,0x80803000},{0x56694,0x005fa12c},
        {0x566ac,0x90903903},{0x566b0,0x029767f0},
        {0x513e0,0x8ca03000},{0x513e4,0x01c00002},
        {0x5d150,0xca91a05a},{0x5d154,0xca91a056},
        {0x6957c,0x82883000},{0x69580,0x005a8831},
        {0x12a4,0x82b03000},{0x12a8,0x005a8831},
        {0x632f0,0x80803000},{0x632f4,0x005a8607},
        {0x63738,0x90483904},{0x6373c,0x005a0224},
        {0x63b98,0x8ca83000},{0x63b9c,0x4091f2e5},
        {0x63c4c,0x8ca03000},{0x63c50,0xbe1ef73c},
        {0x5a8830,0x00000102},{0x5fa12c,21},{0x5a85e8,0x005a85f0},
        {0x5a85fc,0x01000000},{0x5a8600,1},
        {0x5a8604,0x69000001},{0x5a8608,0x96111100},{0x5a860c,0x009d9d96},
        {0x5b4b18,0xf6540000},{0x5b4b1c,3540},{0x5b4a80,5400},
        {0x5b4984,216},{0x5b4a08,uint16_t(-360)}
    };
    NativeFixture() {
        for(auto [address,value]:std::map<uint32_t,float>{
            {0x5a17d4,105},{0x5a17d8,0},{0x5a2230,17},{0x5a2234,17},
            {0x5a021c,75},{0x5a0220,75},{0x5a0224,78.5},{0x5a0228,78.5},
            {0x5b489c,400},{0x5b4aa8,0.428571462631f}})
            words[address]=std::bit_cast<uint32_t>(value);
    }
    auto read(){return [this](uint32_t address){return words[address];};}
};

// Optional private capture verification. No ROM, NVRAM or captured coordinates
// need to be distributed with the public source/unit-test fixture above.
static void verify_capture(const char* path) {
    std::ifstream input(path);assert(input);
    std::string line;std::getline(input,line);
    assert(line.starts_with("time,player,mode,x,y,"));
    NativeFixture memory;std::array<int,2> previous{},mode{};
    size_t checked{},inverted{};double maximum{},maximum_pixels{};
    while(std::getline(input,line)) {
        std::istringstream fields(line);std::string field;std::vector<double> row;
        while(std::getline(fields,field,','))row.push_back(std::stod(field));
        assert(row.size()>=25&&row.size()<=27);const auto p=unsigned(row[1]);assert(p<2);
        if(row[2]==21&&(row.size()==27||mode[p]==21)) {
            if(row.size()==27)assert(std::isfinite(row[26]));
            memory.words[0x5b4984+p*0x84]=uint16_t(row.size()==27?int(row[26]):previous[p]);
            const auto calibration=bel::read_native_calibration(memory.read(),p);
            assert(calibration.valid);
            const auto point=bel::native_aim_point(calibration,uint8_t(row[3]),uint8_t(row[4]));
            assert(point.valid);
            const auto error=std::max({std::abs(point.x-row[18]),std::abs(point.y-row[19]),std::abs(point.z-row[20])});
            maximum=std::max(maximum,error);
            if(error>.000004)std::cerr<<"Native point mismatch at "<<row[0]<<", P"<<p<<": "<<error<<'\n';
            assert(error<.000004);++checked;
            const bel::NativeLens lens{496,384,400,400,249,192};
            const auto screen=lens.project_point({row[18],row[19]-calibration.origin_y,-row[20]});
            const double u=screen.u,v=screen.v;
            if(u>=0&&u<=1&&v>=0&&v<=1) {
                const auto adc=bel::native_screen_to_adc(p,u,v,calibration);assert(adc.valid);
                const auto mapped=bel::native_aim_point(calibration,adc.x,adc.y);assert(mapped.valid);
                const double dx=(mapped.x/mapped.z-row[18]/row[20])*400.;
                const double dy=((mapped.y-calibration.origin_y)/mapped.z-(row[19]-calibration.origin_y)/row[20])*400.;
                const double pixels=std::hypot(dx,dy);
                maximum_pixels=std::max(maximum_pixels,pixels);assert(pixels<.001);++inverted;
            }
        }
        previous[p]=int(row[16]);mode[p]=int(row[2]);
    }
    assert(checked>100);
    std::cout<<"Original native live points checked: "<<checked<<", maximum coordinate error: "<<maximum<<'\n';
    std::cout<<"Original native screen points inverted: "<<inverted<<", maximum pixel error: "<<maximum_pixels<<'\n';
}

static void verify_native_delivery() {
    const bel::NativeLens lens{496,384,400,400,249,192};
    const auto target=lens.project_point(lens.unproject(249,60,1));
    for(int layout=0;layout<2;layout++) {
        NativeFixture memory;memory.words[0x5b4994]=memory.words[0x5b4a18]=0x0a8c0000;
        memory.words[0x5b4984]=0;memory.words[0x5b4a08]=5400;
        bel::SharedRuntime runtime;runtime.update_native(memory.read());
        bel::InputPolicy controls;bel::NativeAimPolicy prior_host_policy;bel::Settings settings;settings.p1_hand=layout;
        bel::Hands hands;
        for(auto& hand:hands){hand.tracked=true;hand.aim={target.u,target.v,true,true};}
        auto queue=[&](double time) {
            auto next=controls.update(time,hands,runtime.state,settings,true,false);
            prior_host_policy.apply(next,hands,runtime.calibration);
            runtime.input=next;
        };
        queue(0);runtime.update_native(memory.read()); // Physical releases arm both guards.
        for(auto& hand:hands)hand.trigger=hand.pump=true;
        queue(.1); // XR calculated against the preceding native snapshot.
        memory.words[0x5b4984]=5400;memory.words[0x5b4a08]=0;
        runtime.update_native(memory.read());
        for(unsigned player=0;player<2;player++) {
            assert(runtime.engine_input.cabinet.trigger[player]&&runtime.engine_input.cabinet.missile[player]);
            const auto point=bel::native_aim_point(runtime.calibration[player],runtime.engine_input.cabinet.x[player],runtime.engine_input.cabinet.y[player]);
            const auto screen=lens.project_point({point.x,point.y-runtime.calibration[player].origin_y,-point.z});
            assert(std::abs(screen.u*495-249)<2&&std::abs(screen.v*383-60)<2);
        }
        // XR can miss an entire native invalid→valid interval. Delivery must
        // still reject the held actions until their own physical releases.
        memory.words[0x5a021c]=0;runtime.update_native(memory.read());
        assert(!runtime.engine_input.cabinet.trigger[0]&&!runtime.engine_input.cabinet.missile[0]);
        memory.words[0x5a021c]=std::bit_cast<uint32_t>(75.f);runtime.update_native(memory.read());
        assert(!runtime.engine_input.cabinet.trigger[0]&&!runtime.engine_input.cabinet.missile[0]);
        for(auto& hand:hands)hand.trigger=hand.pump=false;
        queue(.2);runtime.update_native(memory.read());
        for(auto& hand:hands)hand.trigger=hand.pump=true;
        queue(.3);runtime.update_native(memory.read());
        assert(runtime.engine_input.cabinet.trigger[0]&&runtime.engine_input.cabinet.missile[0]);
        memory.words[0x5a8830]=0x0000ff02;runtime.update_native(memory.read());
        assert(!runtime.engine_input.cabinet.trigger[0]&&!runtime.engine_input.cabinet.missile[0]);
        bel::ServiceGunPolicy service;bel::Hands operator_hands;
        for(auto& hand:operator_hands){hand.tracked=true;hand.aim={0,1,true,true};}
        service.update(operator_hands,settings,true);
        for(auto& hand:operator_hands)hand.trigger=true;
        runtime.input=service.update(operator_hands,settings,true);runtime.update_native(memory.read());
        assert(runtime.engine_input.cabinet.trigger[0]&&runtime.engine_input.cabinet.y[0]==17);
        memory.words[0x5a8830]=0x00000102;runtime.update_native(memory.read());
        assert(!runtime.engine_input.cabinet.trigger[0]); // Queued service input cannot become gameplay.
        queue(.4);runtime.update_native(memory.read());
        assert(!runtime.engine_input.cabinet.trigger[0]&&!runtime.engine_input.cabinet.missile[0]);
        for(auto& hand:hands)hand.trigger=hand.pump=false;
        queue(.5);runtime.update_native(memory.read());
        for(auto& hand:hands)hand.trigger=hand.pump=true;
        queue(.6);runtime.update_native(memory.read());
        assert(runtime.engine_input.cabinet.trigger[0]);
        // Native ownership changes invalidate a queued XR assignment.
        memory.words[0x5b4994]=0;runtime.update_native(memory.read());
        assert(!runtime.engine_input.cabinet.trigger[0]&&!runtime.engine_input.cabinet.trigger[1]);
        memory.words[0x5b4994]=0x0a8c0000;
        runtime.input.cabinet.coins=runtime.input.cabinet.starts=1;
        memory.words[0x5fa12c]=11;runtime.update_native(memory.read());
        memory.words[0x5fa12c]=21;runtime.update_native(memory.read());
        assert(!runtime.engine_input.cabinet.coins&&!runtime.engine_input.cabinet.starts);
        assert(!runtime.engine_input.cabinet.trigger[0]); // Old-session intent stays rejected after world returns.
    }
}

int main(int argc,char** argv) {
    NativeFixture memory;
    const auto p1=bel::read_native_calibration(memory.read(),0);
    const auto p2=bel::read_native_calibration(memory.read(),1);
    assert(p1.valid&&p2.valid&&!p1.factory&&!p2.factory);
    assert(p1.left==105&&p1.right==255&&p1.top==174&&p1.bottom==17);
    assert(p2.left==0&&p2.right==150&&p2.top==174&&p2.bottom==17);
    // Independent live original TGP samples, not values computed by the helper.
    const auto point1=bel::native_aim_point(p1,177,95);
    assert(point1.valid&&std::abs(point1.x-(-0.0944434329867))<.000004);
    assert(std::abs(point1.y-(-0.167044833302))<.000004);
    assert(std::abs(point1.z-4.55990695953)<.000004);
    const auto point2=bel::native_aim_point(p2,80,95);
    assert(point2.valid&&std::abs(point2.x-0.157383710146)<.000004);
    assert(std::abs(point2.y-(-0.167044833302))<.000004);
    assert(std::abs(point2.z-4.55816888809)<.000004);
    // Native lower pitch clamp is asymmetric, independent of ADC endpoint.
    const auto lower=bel::native_aim_point(p1,105,17);
    assert(lower.valid&&std::abs(lower.x-(-2.19388580322))<.000004);
    assert(std::abs(lower.y-(-1.22778177261))<.000004);
    assert(std::abs(lower.z-3.85205483437)<.000004);
    assert(!bel::native_aim_point(p1,104,95).valid);
    assert(!bel::native_aim_point(p1,180,175).valid);
    assert(!bel::read_native_calibration(memory.read(),2).valid);
    // Original per-player firing geometry in native frame 1727 (focus400,
    // center249/192, image496x384), independently captured before inversion.
    assert(p1.screen_inverse_valid);
    const bel::NativeLens lens{496,384,400,400,249,192};
    const auto screen1=lens.project_point({-.094443433,-.0118048508,-4.55990696});
    const auto inverse=bel::native_screen_to_adc(0,screen1.u,screen1.v,p1);
    assert(inverse.valid&&inverse.x==177&&inverse.y==95);
    const auto screen2=lens.project_point({.157383710146,-.011804834007719,-4.55816888809});
    const auto inverse2=bel::native_screen_to_adc(1,screen2.u,screen2.v,p2);
    assert(inverse2.valid&&inverse2.x==80&&inverse2.y==95);
    const auto between_pixels=lens.project_point(lens.unproject(241.9,193.03,1));
    const auto nearest=bel::native_screen_to_adc(0,between_pixels.u,between_pixels.v,p1);
    assert(nearest.valid&&nearest.x==177&&nearest.y==95);
    assert(!bel::native_screen_to_adc(0,0,.5,p1).valid); // Outside native gun stops.
    assert(!bel::native_screen_to_adc(0,.5,.95,p1).valid); // Lower native HUD.
    assert(!bel::native_screen_to_adc(2,.5,.5,p1).valid);
    auto invalid_calibration=p1;invalid_calibration.previous_yaw=std::nanf("");
    assert(!bel::native_aim_point(invalid_calibration,177,95).valid);
    assert(!bel::native_screen_to_adc(0,.5,.5,invalid_calibration).valid);
    {
        bel::NativeAimPolicy guard;bel::Hands hands;
        hands[0].tracked=true;hands[0].pump=true;
        hands[0].aim={screen1.u,screen1.v,true,true};
        bel::CabinetInput in;in.hand={0,-1};in.cabinet.missile[0]=true;
        std::array<bel::NativeAimCalibration,2> calibration{};
        guard.apply(in,hands,calibration);assert(!in.cabinet.missile[0]);
        calibration[0]=p1;in.cabinet.missile[0]=true;
        guard.apply(in,hands,calibration);
        assert(!in.cabinet.missile[0]); // Release main trigger cannot release Missile guard.
        hands[0].pump=false;guard.apply(in,hands,calibration);
        hands[0].pump=true;in.cabinet.missile[0]=true;
        guard.apply(in,hands,calibration);assert(in.cabinet.missile[0]);
    }
    // Virgin EEPROM FF, zero/wrapped spans, stale converted data, wrong ROM
    // and non-world/service states must not authorize the native point adapter.
    memory.words[0x5a8608]=0xffffffff;
    assert(!bel::read_native_calibration(memory.read(),0).valid);
    memory=NativeFixture{};memory.words[0x5a8608]&=0x00ffffff;
    assert(!bel::read_native_calibration(memory.read(),0).valid);
    memory=NativeFixture{};memory.words[0x5a0224]=std::bit_cast<uint32_t>(77.f);
    assert(!bel::read_native_calibration(memory.read(),0).valid);
    memory=NativeFixture{};memory.words[0x632f4]=0;
    assert(!bel::read_native_calibration(memory.read(),0).valid);
    memory=NativeFixture{};memory.words[0x5fa12c]=4;
    assert(!bel::read_native_calibration(memory.read(),0).valid);
    memory=NativeFixture{};memory.words[0x5a8830]=0x0000ff02;
    assert(!bel::read_native_calibration(memory.read(),0).valid);
    verify_native_delivery();
    if(argc>1)verify_capture(argv[1]);
}
