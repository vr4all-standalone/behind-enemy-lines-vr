#include "../native/gun_setup.h"
#include <cassert>
using namespace bel;
int main() {
    GameState s;s.valid=s.ready=s.calibration_erased=true;
    {
        GunSetupPolicy paused;
        assert(paused.update(s,true).cabinet.test);
        auto released=paused.update(s,false);
        assert(released.active&&!released.cabinet.test&&!released.cabinet.service);
        for(int i=0;i<5000;i++)assert(paused.update(s,false).active);
        assert(!paused.failed()); // Suspension does not consume the timeout.
    }
    GunSetupPolicy setup;
    auto step=setup.update(s,false);assert(!step.active); // No host authorization.
    s.players[0].state=1;step=setup.update(s,true);assert(!step.active);
    s.players[0].state=0;step=setup.update(s,true);
    assert(step.active&&step.cabinet.test&&!step.cabinet.coins&&!step.cabinet.starts);
    s.service=true;s.service_selection=10;
    bool advanced=false;
    for(int i=0;i<100;i++){step=setup.update(s,true);advanced|=step.cabinet.service;assert(step.active);}
    assert(advanced);
    s.service_selection=3;bool selected=false;
    for(int i=0;i<100;i++){step=setup.update(s,true);selected|=step.cabinet.test;}
    assert(selected);
    s.gun_calibration=true;s.service_menu=3;
    for(int sub:{3,5,7,9}) {
        s.service_submode=sub;bool fired=false;
        // A previously accepted native press finishes its held/released cycle
        // before the next stop gets a fresh edge.
        for(int i=0;i<36;i++)setup.update(s,true);
        for(int i=0;i<100;i++) {
            step=setup.update(s,true);
            for(int p=0;p<2;p++)if(step.cabinet.trigger[p]) {
                assert(p==(sub>=7?1:0));
                const bool upper=sub==5||sub==9;
                assert(step.cabinet.x[p]==(upper?NativeRanges[p].right:NativeRanges[p].left));
                assert(step.cabinet.y[p]==(upper?NativeRanges[p].top:NativeRanges[p].bottom));
                fired=true;
            }
        }
        assert(fired);
    }
    s.service_submode=11;bool exit_guns=false;
    for(int i=0;i<100;i++){step=setup.update(s,true);exit_guns|=step.cabinet.test;}
    assert(exit_guns);
    s.gun_calibration=false;s.service_menu=0;s.service_selection=10;
    bool saved=false;for(int i=0;i<100;i++){step=setup.update(s,true);saved|=step.cabinet.test;}
    assert(saved);
    s.valid=false;s.service=false;for(int i=0;i<30;i++)assert(setup.update(s,true).active);
    s.valid=s.ready=s.calibration_config_valid=true;s.calibration_erased=false;
    assert(!setup.update(s,true).active&&setup.completed());
    // Existing native calibration is never overwritten, even if a particular
    // later camera mode cannot be inverted.
    GunSetupPolicy existing;assert(!existing.update(s,true).active);
    s.calibration_config_valid=false;assert(!existing.update(s,true).active);
    GunSetupPolicy timeout;s.calibration_erased=true;
    for(int i=0;i<5000;i++)step=timeout.update(s,true);
    assert(timeout.failed()&&!step.active&&!step.cabinet.test&&!step.cabinet.service);
    assert(!step.cabinet.trigger[0]&&!step.cabinet.trigger[1]);
}
