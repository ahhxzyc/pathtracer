#pragma once

#include "common.h"
#include "bsdf.h"
#include "light.h"
#include "material.h"
#include <optional>
#include <memory>

class Primitive;
class Bound3f;

struct Intersection
{
    bool backface = false;
    float t;
    Point3f point;
    Vec3f normal;
    Point2f uv;
    Vec3f wo;
    const GeometricPrimitive* primitive;
    BSDF bsdf;
    void BuildBSDF();
};

struct PrimitiveSample
{
    Point3f point;
    Vec3f normal;
    float pdf;
};

class Primitive
{
public:
    virtual ~Primitive() {}
    virtual std::optional<Intersection> intersect(Ray &ray) const = 0;
    virtual bool has_intersection(const Ray &ray) const = 0;
    virtual Bound3f BoundingBox() const = 0;
    virtual Point3f Center() const = 0;
};

class GeometricPrimitive : public Primitive
{
public:
    GeometricPrimitive(
        const std::shared_ptr<Material> &material);
    const Material*     GetMaterial() const { return m_Material.get(); }
    const AreaLight*    GetAreaLight() const { return m_AreaLight.get(); }
    void                SetAreaLight(const std::shared_ptr<AreaLight>& light) {m_AreaLight = light;}
    virtual PrimitiveSample sample() const = 0;
    virtual float           area() const = 0;
protected:
    std::shared_ptr<Material> m_Material;
    std::shared_ptr<AreaLight> m_AreaLight;
};

class Triangle : public GeometricPrimitive
{
public:
    Triangle(const std::shared_ptr<Material> &material) : 
        GeometricPrimitive(material) {}
    virtual std::optional<Intersection> intersect(Ray &ray) const override;
    virtual bool has_intersection(const Ray &ray) const override;
    virtual PrimitiveSample             sample() const override;
    virtual Bound3f BoundingBox() const override;
    virtual Point3f Center() const override;
    virtual float area() const override;
public:
    Vec3f p[3];
    Vec3f n[3];
    Vec2f uv[3];
};