#include "Aggregate.h"
#include <algorithm>

//
//OctreeAggregate::OctreeAggregate(const std::vector<std::shared_ptr<Primitive>> &primitives)
//{
//    Bound3f box;
//    for (const auto &p : primitives)
//        box = box.merge(p->GetBoundingBox());
//    m_Root = BuildRecursive(box, primitives);
//}
//
//OctreeNode *OctreeAggregate::BuildRecursive(const Bound3f &box, const std::vector<std::shared_ptr<Primitive>> &primitives)
//{
//    if (primitives.empty())
//        return nullptr;
//
//    auto node = new OctreeNode();
//    node->box = box;
//    if (primitives.size() < 5)
//    {
//        node->primitives = primitives;
//        return node;
//    }
//    vector<int> taken(primitives.size(), false);
//    for (int i = 0; i < 8; i ++)
//    {
//        vector<Triangle> sub_tris;
//        Bound3f sub = box.getSubBox(i);
//        for (int j = 0; j < primitives.size(); j ++)
//        {
//            if (taken[j])
//                continue;
//            if (sub.contain(primitives[j]))
//            {
//                sub_tris.push_back(tris[j]);
//                taken[j] = true;
//            }
//        }
//        node->children[i] = build(sub, sub_tris);
//    }
//    for (int i = 0; i < primitives.size(); i ++)
//    {
//        if (!taken[i])
//            node->primitives.push_back(primitives[i]);
//    }
//    return node;
//}
//
//std::optional<Intersection> OctreeNode::Intersect(const Ray &ray, float tmin, float tmax) const
//{
//    if (!box.intersect(ray, tmin, tmax))
//    {
//        return {};
//    }
//
//    Intersection is;
//    for (const auto &p : primitives)
//    {
//        auto tis = p->Intersect(ray, tmin, tmax);
//        if (tis)
//        {
//            tmax = tis->t;
//            is = *tis;
//    }
//
//    for (int i = 0; i < 8; i ++)
//    {
//        if (!children[i])
//            continue;
//        auto tis = children[i]->Intersect(ray, tmin, tmax);
//        if (tis)
//        {
//            tmax = tis->t;
//            is = *tis;
//        }
//    }
//    return {is};
//}

std::optional<Intersection> ListAggregate::Intersect(const Ray &ray, float tmin, float tmax) const
{
    std::optional<Intersection> ret = {};
    for (const auto &p : m_Primitives)
    {
        auto is = p->Intersect(ray, tmin, tmax);
        if (is)
        {
            tmax = is->t;
            ret.emplace(*is);
        }
    }
    return ret;
}

std::optional<Intersection> BVHAggregate::Intersect(const Ray &ray, float tmin, float tmax) const
{
    BVHNode* nodeStack[64];
    int top = -1;
    nodeStack[ ++ top] = m_Root;
    std::optional<Intersection> is = {};
    while (top >= 0)
    {
        auto node = nodeStack[top -- ];

        // Check intersection with node's bounding box
        if (!node->box.ExistIntersection(ray, tmin, tmax))
        {
            continue;
        }

        if (!node->children[0])
        {
            // Find intersection with leaf node's primitives
            for (int i = node->startIndex; i < node->startIndex + node->primitiveCount; i ++)
            {
                auto tis = m_Primitives[i]->Intersect(ray, tmin, tmax);
                if (tis)
                {
                    tmax = tis->t;
                    is = tis;
                }
            }
        }
        else
        {
            // Push children into the node stack to visit later
            nodeStack[ ++ top] = node->children[0];
            nodeStack[ ++ top] = node->children[1];
        }
    }

    return is;
}

BVHNode *BVHAggregate::BuildRecursive(int start, int n)
{
    // Create new node
    auto node = new BVHNode;

    // Compute bounding box
    Bound3f box;
    for (int i = start; i < start + n; i ++)
    {
        box = box.Union(m_Primitives[i]->BoundingBox());
    }
    
    if (n <= 5)
    {
        // Create leaf node
        node->startIndex = start;
        node->primitiveCount = n;
        node->box = box;
    }
    else
    {
        // Create interior node
        
        // Compute bounding box of primitive centroids
        Bound3f centroidBox;
        for (int i = start; i < start + n; i ++)
        {
            centroidBox = centroidBox.Union(m_Primitives[i]->Center());
        }

        // Decide split axis and split location
        int dim = centroidBox.MaxExtent();
        auto midPoint = (centroidBox.m_max[dim] + centroidBox.m_min[dim]) / 2.f;

        // Partition the primitive array
        auto midIter = std::partition(
            m_Primitives.begin() + start, m_Primitives.begin() + start + n,
            [dim, midPoint](const std::shared_ptr<Primitive> &primitive)
            {
                return primitive->Center()[dim] < midPoint;
            });
        auto mid = static_cast<int>(midIter - m_Primitives.begin());
        
        // Build children nodes
        node->children[0] = BuildRecursive(start, mid - start);
        node->children[1] = BuildRecursive(mid, n - (mid - start));
        node->box = box;
    }
    
    return node;
}


bool BVHAggregate::ExistIntersection(const Ray &ray, float tmin, float tmax) const
{
    BVHNode *nodeStack[64];
    int top = -1;
    nodeStack[++ top] = m_Root;
    std::optional<Intersection> is = {};
    while (top >= 0)
    {
        auto node = nodeStack[top --];

        // Check intersection with node's bounding box
        if (!node->box.ExistIntersection(ray, tmin, tmax))
        {
            continue;
        }

        if (!node->children[0])
        {
            // Find intersection with leaf node's primitives
            for (int i = node->startIndex; i < node->startIndex + node->primitiveCount; i ++)
            {
                if (m_Primitives[i]->ExistIntersection(ray, tmin, tmax))
                {
                    return true;
                }
            }
        }
        else
        {
            // Push children into the node stack to visit later
            nodeStack[++ top] = node->children[0];
            nodeStack[++ top] = node->children[1];
        }
    }

    return false;
}

Bound3f BVHAggregate::BoundingBox() const
{
    return m_Root ? m_Root->box : Bound3f();
}

Point3f BVHAggregate::Center() const
{
    return BoundingBox().Center();
}
