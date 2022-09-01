#pragma once

#include "Primitive.h"

class Aggregate : public Primitive
{
public:
    Aggregate() {}
    virtual ~Aggregate() {}
};

class ListAggregate : public Aggregate
{
public:
    ListAggregate(const std::vector<std::shared_ptr<Primitive>> &primitives) : m_Primitives(primitives) {}
    virtual ~ListAggregate() {}

    virtual std::optional<Intersection> 
        Intersect(const Ray &ray, float tmin, float tmax) const override;

private:
    std::vector<std::shared_ptr<Primitive>> m_Primitives;
};

struct OctreeNode : public Aggregate
{
    Bound3f box;
    std::vector<std::shared_ptr<Primitive>> primitives;
    OctreeNode *children[8] = { nullptr };

    virtual std::optional<Intersection> 
        Intersect(const Ray &ray, float tmin, float tmax) const override;

};
class OctreeAggregate : Aggregate
{
public:
    OctreeAggregate(const std::vector<std::shared_ptr<Primitive>> &primitives);
    virtual ~OctreeAggregate();
    
    static OctreeNode *BuildRecursive(
        const Bound3f& box, const std::vector<std::shared_ptr<Primitive>> &primitives);


private:
    OctreeNode *m_Root {nullptr};
};
