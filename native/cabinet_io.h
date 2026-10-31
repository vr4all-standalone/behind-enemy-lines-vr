// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
namespace bel {
// MAME 0.289 model2.cpp: bel -> gunblade -> rchase2 -> model2crx.
// BEL inherits Gunblade's unreversed ADC fields, swaps Test/Service, and
// adds one native Missile button per player on IN1.
struct Cabinet {
    std::array<uint8_t,2> x{0xb1,0x50},y{0x5f,0x5f};
    std::array<bool,2> trigger{};
    std::array<bool,2> missile{};
    uint8_t coins{},starts{};
    bool test{},service{};
};
inline uint8_t in0(uint8_t native,const Cabinet& input) {
    uint8_t active=(input.coins&3)|((input.starts&3)<<4);
    if(input.test)active|=8;
    if(input.service)active|=4;
    return native&~active;
}
inline uint8_t in1(uint8_t native,const Cabinet& input) {
    return native&~((input.trigger[0]?1:0)|(input.trigger[1]?2:0)|
                   (input.missile[0]?0x10:0)|(input.missile[1]?0x20:0));
}
inline uint8_t adc(unsigned channel,const Cabinet& input) {
    switch(channel) {
    case 0:return input.x[0];case 1:return input.x[1];
    case 2:return input.y[0];case 3:return input.y[1];
    default:return 0xff;
    }
}
struct Calibration {uint8_t left{},right{},top{},bottom{};};
// Screen top/bottom follow BEL's measured native positive-Y calibration.
// This does not reverse the hardware ADC channels or raw port limits.
inline constexpr std::array<Calibration,2> NativeRanges{{{0x69,0xff,0xae,0x11},{0x00,0x96,0xae,0x11}}};
struct AimValue {uint8_t x{},y{};bool valid{};};
inline AimValue map_aim(unsigned player,double u,double v,Calibration calibration) {
    if(player>1 || !std::isfinite(u) || !std::isfinite(v) ||
       calibration.left==calibration.right || calibration.top==calibration.bottom)return {};
    const auto interpolate=[](double amount,uint8_t from,uint8_t to) {
        return uint8_t(std::lround(from+std::clamp(amount,0.,1.)*(int(to)-int(from))));
    };
    return {interpolate(u,calibration.left,calibration.right),interpolate(v,calibration.top,calibration.bottom),true};
}
inline AimValue map_aim(unsigned player,double u,double v) {
    if(player>1)return {};
    return map_aim(player,u,v,NativeRanges[player]);
}
}
