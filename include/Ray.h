#pragma once

#include "Types.h"
#include "Intersection.h"

struct Ray {
    // origin of the ray
    Vec3f orig;
    // the direction where the ray is cast, should always be unit vector
    Vec3f dir;

    Ray(Vec3f o, Vec3f d) {
        dir = d.normalized();
        orig = o + dir * 0.01;
    }


    // reflect after an intersection
    Ray reflect(Intersection inter) {
        Vec3f ref = dir + 2 * inter.normal * inter.normal.dot(-dir);
        return Ray(
                inter.point,
                ref
        );
    }
};