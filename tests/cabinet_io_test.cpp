// SPDX-License-Identifier: GPL-2.0-or-later
#include "../native/cabinet_io.h"
#include <cassert>
#include <limits>
int main() {
    bel::Cabinet c;
    c.test=true;
    assert(bel::in0(0xff,c)==0xf7); // BEL native Test is bit 0x08.
    c.test=false;c.service=true;
    assert(bel::in0(0xff,c)==0xfb); // BEL native Service is bit 0x04.
    c={};c.coins=3;c.starts=3;
    assert(bel::in0(0xff,c)==0xcc);
    c={};c.trigger={true,true};
    assert(bel::in1(0xff,c)==0xfc);
    c.x={0x69,0x96};c.y={0x11,0xae};
    assert(bel::adc(0,c)==0x69 && bel::adc(1,c)==0x96);
    assert(bel::adc(2,c)==0x11 && bel::adc(3,c)==0xae);
    assert(bel::adc(4,c)==0xff);
    // Native calibration saves positive Y spans: the upper stop is AE and
    // the lower stop is 11. Screen-v increases downward in the VR adapter.
    auto a=bel::map_aim(0,0,0);assert(a.valid&&a.x==0x69&&a.y==0xae);
    a=bel::map_aim(0,1,1);assert(a.valid&&a.x==0xff&&a.y==0x11);
    a=bel::map_aim(1,0,0);assert(a.valid&&a.x==0&&a.y==0xae);
    a=bel::map_aim(1,1,1);assert(a.valid&&a.x==0x96&&a.y==0x11);
    assert(!bel::map_aim(2,.5,.5).valid);
    assert(!bel::map_aim(0,std::numeric_limits<double>::quiet_NaN(),.5).valid);
}
