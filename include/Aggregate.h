#pragma once

#include "Primitive.h"
#include "Bound3f.h"

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

struct BVHNode
{
    Bound3f box;
    int startIndex, primitiveCount;
    BVHNode *children[2] {nullptr};
};

class BVHAggregate : public Aggregate
{
public:
    BVHAggregate(const std::vector<std::shared_ptr<Primitive>> &primitives)
        : m_Primitives(primitives)
    {
        m_Root = BuildRecursive(0, primitives.size());
    }

    virtual std::optional<Intersection> Intersect(const Ray &ray, float tmin, float tmax) const override;
    virtual bool                        ExistIntersection(const Ray &ray, float tmin, float tmax) const override;
    virtual Bound3f                     BoundingBox() const override;

    BVHNode *BuildRecursive(int startIndex, int primitiveCount);



    Point3f Center() const override;

private:
    std::vector<std::shared_ptr<Primitive>> m_Primitives;
    BVHNode *m_Root {nullptr};
};