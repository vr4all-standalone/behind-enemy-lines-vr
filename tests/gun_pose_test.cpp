// SPDX-License-Identifier: GPL-2.0-or-later
#include "../native/gun_pose.h"
#include <cassert>
#include <limits>
int main() {
    auto direction=[](float angle){return glm::vec3(bel::pitched_gun_pose(glm::mat4(1),angle)*glm::vec4(0,0,-1,0));};
    assert(std::abs(direction(0).y-(-.17364818f))<.000001f);
    assert(std::abs(direction(30).y-.34202014f)<.000001f);
    assert(glm::length(direction(99)-direction(30))<.000001f);
    assert(glm::length(direction(std::numeric_limits<float>::quiet_NaN())-direction(0))<.000001f);
    auto aim=glm::translate(glm::mat4(1),glm::vec3(0,0,-.1f));
    auto anchored=bel::gun_pose(aim,glm::vec3(1,2,3));
    assert(glm::vec3(anchored[3])==glm::vec3(1,2,3));
    assert(anchored[0]==aim[0]);
    auto left=bel::gun_model_transform(anchored,1,0,glm::mat4(1))*glm::vec4(.02f,0,-.1f,1);
    auto right=bel::gun_model_transform(anchored,1,1,glm::mat4(1))*glm::vec4(.02f,0,-.1f,1);
    assert(std::abs(left.x-.98f)<.000001f&&std::abs(right.x-1.02f)<.000001f);
    assert(left.z==right.z);
}
