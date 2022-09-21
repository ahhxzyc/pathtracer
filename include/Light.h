#pragma once

#include "Types.h"
#include "Ray.h"

class GeometricPrimitive;
class Intersection;

struct LightSample
{
    // information with respect to global coordinates
    Point3f point;
    Vec3f wi;
    Color3f Le;
    float pdf;
};

class Light
{
public:
    virtual LightSample Sample(const Point3f &point) const = 0;
    virtual float Pdf(const Intersection &is, const Vec3f &wi) const = 0;
    virtual Color3f Radiance(const Ray &ray) const = 0;
    virtual Point3f Center() const = 0;
};


class AreaLight : public Light
{
public:
    AreaLight(const GeometricPrimitive& primitive, const Color3f &radiance) : 
        m_Primitive(primitive), m_Radiance(radiance) { }

    Color3f L() const {return m_Radiance;}

    virtual LightSample Sample(const Point3f &point) const override;
    virtual float Pdf(const Intersection &is, const Vec3f &wi) const override;
    virtual Color3f Radiance(const Ray &ray) const override;
    virtual Point3f Center() const override;
private:
    const GeometricPrimitive &m_Primitive ;
    Color3f m_Radiance;
};