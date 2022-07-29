#pragma once

#include "Types.h"

struct Material {
    Vec3f kd = Vec3f(0, 0, 0);
    Vec3f ka = Vec3f(0, 0, 0);
    Vec3f ks = Vec3f(0, 0, 0);
    Vec3f ke = Vec3f(0, 0, 0);
};