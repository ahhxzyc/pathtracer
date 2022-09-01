#pragma once

#include "Types.h"
#include "Ray.h"
#include "Bound3f.h"
#include <optional>
#include <memory>

class Primitive;
struct Intersection
{
    bool backface = false;
    float t;
    Point3f point;
    Point3f bary;
    Vec3f normal;
    const Primitive* primitive;
};


class Primitive
{
public:
    virtual ~Primitive() {}

    auto GetBoundingBox() const {return m_Bound;}

    virtual std::optional<Intersection> Intersect(const Ray &ray, float tmin, float tmax) const = 0;

protected:
    Bound3f m_Bound;
};

class Triangle : public Primitive
{
public:
    Triangle() {}
    Triangle(const array<Vec3f, 3> &verts)
    {
        for (int i = 0; i < 3; i ++)
            p[i] = verts[i];
    }

    virtual std::optional<Intersection> Intersect(const Ray &ray, float tmin, float tmax) const;

    //float area() const;
    //Vec3f center() const;
    //Vec3f normal() const;

public:
    Vec3f p[3];
    Vec3f n[3];
    Vec2f t[3];
    std::shared_ptr<Material> mMaterial;
};