#pragma once

#include "Types.h"
#include "Material.h"

#include <limits>

class Triangle;



struct Ray {
    Vec3f orig;
    Vec3f dir;

    Ray(Vec3f o, Vec3f d) 
        : orig(o), dir(glm::normalize(d))
    {

    }

    //// reflect after an intersection
    //Ray reflect(const Intersection &inter) {
    //    Vec3f ref = dir + 2.f * inter.normal * glm::dot(inter.normal, -dir);
    //    return Ray( inter.point, ref );
    //}
};