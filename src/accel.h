#pragma once

#include "primitive.h"
#include "Bound3f.h"

class Accel : public Primitive
{
public:
    virtual ~Accel() {}
};

class ListAccel : public Accel
{
public:
    ListAccel(const std::vector<std::shared_ptr<Primitive>> &primitives) : primitives_(primitives) {}

    std::optional<Intersection> intersect(Ray &ray) const override;
    bool                        has_intersection(const Ray &ray) const override;
    Bound3f                     BoundingBox() const override;
    Point3f Center() const override;

private:
    std::vector<std::shared_ptr<Primitive>> primitives_;
};

struct BVHNode
{
    Bound3f box;
    int startIndex, primitiveCount;
    BVHNode *children[2] {nullptr};
};

class BVHAccel : public Accel
{
public:
    BVHAccel(const std::vector<std::shared_ptr<Primitive>> &primitives)
        : primitives_(primitives)
    {
        m_Root = build_node(0, primitives.size());
    }

    std::optional<Intersection> intersect(Ray &ray) const override;
    bool                        has_intersection(const Ray &ray) const override;
    Bound3f                     BoundingBox() const override;

    BVHNode *build_node(int startIndex, int primitiveCount);

    Point3f Center() const override;

private:
    std::vector<std::shared_ptr<Primitive>> primitives_;
    BVHNode *m_Root {nullptr};
};