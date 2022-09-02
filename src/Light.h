#pragma once

#include "Types.h"

class GeometricPrimitive;

struct LightSample
{
    Point3f point;
    Vec3f direction;
    Color3f radiance;
    float pdf;
};

class Light
{
public:
    virtual LightSample Sample(const Point3f &point) const = 0;
};


class AreaLight : public Light
{
public:
    AreaLight(const GeometricPrimitive& primitive, const Color3f &radiance) : 
        m_Primitive(primitive), m_Radiance(radiance) { }

    Color3f L() const {return m_Radiance;}

    virtual LightSample Sample(const Point3f &point) const override;

private:
    const GeometricPrimitive &m_Primitive ;
    Color3f m_Radiance;
};