#pragma once

#include "Types.h"
#include "Ray.h"

class Bound3f
{
public:
    Bound3f();
    Bound3f(Vec3f minp, Vec3f maxp);

    Bound3f Union(Bound3f box) const;
    Bound3f Union(Point3f point) const;

    int MaxExtent() const;

    //// return one of the 8 sub-boxes;
    //Bound3f getSubBox(int idx) const;
    
    bool contain(Vec3f p);

    bool ExistIntersection(const Ray &ray, float tmin, float tmax) const;
    Point3f Center() const;

public:
    Vec3f m_min;
    Vec3f m_max;
};