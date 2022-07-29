#pragma once

#include "Types.h"
#include "Intersection.h"
#include "Triangle.h"
#include "Ray.h"

class AABB {
public:
    AABB();
    AABB(Vec3f minp, Vec3f maxp);
    AABB(Triangle tri);

    // merge two AABBs
    AABB merge(AABB box);

    // intersection with a ray
    bool intersect(Ray ray);
    // return one of the 8 sub-boxes;
    AABB getSubBox(int idx);
    
    bool contain(Vec3f p);
    bool contain(Triangle tri);

public:
    Vec3f m_min;
    Vec3f m_max;

};