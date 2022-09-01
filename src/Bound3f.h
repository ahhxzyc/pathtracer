#pragma once

#include "Types.h"
#include "Intersection.h"
#include "Triangle.h"
#include "Ray.h"

class Bound3f {
public:
    Bound3f();
    Bound3f(Vec3f minp, Vec3f maxp);
    Bound3f(Triangle tri);

    // merge two AABBs
    Bound3f merge(Bound3f box);

    // intersection with a ray
    bool intersect(Ray ray, float tmin, float tmax)const;
    // return one of the 8 sub-boxes;
    Bound3f getSubBox(int idx) const;
    
    bool contain(Vec3f p);
    bool contain(Triangle tri);

public:
    Vec3f m_min;
    Vec3f m_max;

};