// SPDX-License-Identifier: GPL-2.0-or-later
#include "../native/controller_controls.h"
#include <cassert>
using namespace bel;
int main() {
    Hands hands;hands[0].tracked=hands[1].tracked=true;
    ExtraControls axes;OperatorPolicy policy;
    policy.update(0,axes,hands,true);
    hands[0].trigger=true;
    assert(policy.update(.1,axes,hands,true)==8); // BEL Test selects.
    hands[0].trigger=false;policy.update(.2,axes,hands,true);
    hands[1].pump=true;
    assert(policy.update(.3,axes,hands,true)==4); // BEL Service advances.
    policy.update(.4,axes,hands,false);
    assert(policy.update(.5,axes,hands,true)==0); // Fresh release after focus.
    {
        Settings settings;ServiceGunPolicy guns;Hands h;
        h[0].tracked=h[1].tracked=true;
        h[0].aim={0,1,true,true};h[1].aim={1,0,true,true};
        guns.update(h,settings,true);
        h[0].trigger=h[1].trigger=true;
        const auto input=guns.update(h,settings,true);
        assert(input.cabinet.trigger[0]&&input.cabinet.trigger[1]);
        assert(input.cabinet.x[0]==0x69&&input.cabinet.y[0]==0x11);
        assert(input.cabinet.x[1]==0x96&&input.cabinet.y[1]==0xae);
    }
}
