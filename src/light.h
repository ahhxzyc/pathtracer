#pragma once

#include "common.h"

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
    virtual LightSample Sample(const Intersection &ref) const = 0;
    virtual float Pdf(const Point3f &refPoint, const Vec3f &lightPoint, const Vec3f &lightNormal) const = 0;
    virtual Color3f Radiance(const Ray &ray) const = 0;
    virtual Point3f Center() const = 0;
};


class AreaLight : public Light
{
public:
    AreaLight(const GeometricPrimitive& primitive, const Color3f &radiance) : 
        primitive_(primitive), radiance(radiance) { }

    Color3f L() const {return radiance;}

    virtual LightSample Sample(const Intersection &ref) const override;
    virtual float Pdf(const Point3f &refPoint, const Vec3f &lightPoint, const Vec3f &lightNormal) const override;
    virtual Color3f Radiance(const Ray &ray) const override;
    virtual Point3f Center() const override;
public:
    Color3f radiance;
private:
    const GeometricPrimitive &primitive_ ;
};