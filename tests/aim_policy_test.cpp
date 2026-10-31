// SPDX-License-Identifier: GPL-2.0-or-later
#include "../native/aim_policy.h"
#include <cassert>
int main() {
    bel::Hands hands;hands[0].tracked=true;hands[0].trigger=hands[0].pump=true;
    hands[0].aim={.5,.5,true,true};
    bel::CabinetInput in;in.hand={0,-1};in.cabinet.trigger[0]=in.cabinet.missile[0]=true;
    in.accepted_shots[0]=1;
    bel::NativeAimPolicy policy;std::array<bel::NativeAimCalibration,2> unverified{};
    policy.apply(in,hands,unverified);
    assert(!in.cabinet.trigger[0]&&!in.cabinet.missile[0]&&in.accepted_shots[0]==0);
}
