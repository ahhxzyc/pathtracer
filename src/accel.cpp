#include "Accel.h"
#include <algorithm>

std::optional<Intersection> ListAccel::intersect(Ray &ray) const
{
    std::optional<Intersection> ret = {};
    for (const auto &p : primitives_)
    {
        auto is = p->intersect(ray);
        if (is)
        {
            ray.maxT = is->t;
            ret.emplace(*is);
        }
    }
    return ret;
}

std::optional<Intersection> BVHAccel::intersect(Ray &ray) const
{
    BVHNode* nodeStack[64];
    int top = -1;
    nodeStack[ ++ top] = m_Root;
    std::optional<Intersection> is = {};
    while (top >= 0)
    {
        auto node = nodeStack[top -- ];

        if (!node->box.ExistIntersection(ray))
        {
            continue;
        }

        if (!node->children[0])
        {
            // this is a leaf node, find intersection with primitives
            for (int i = node->startIndex; i < node->startIndex + node->primitiveCount; i ++)
            {
                auto tis = primitives_[i]->intersect(ray);
                if (tis)
                {
                    ray.maxT = tis->t;
                    is = tis;
                }
            }
        }
        else
        {
            // push children into the node stack to visit later
            nodeStack[ ++ top] = node->children[0];
            nodeStack[ ++ top] = node->children[1];
        }
    }

    return is;
}

BVHNode *BVHAccel::build_node(int start, int n)
{
    // Create new node
    auto node = new BVHNode;

    // Compute bounding box
    Bound3f box;
    for (int i = start; i < start + n; i ++)
    {
        box = box.Union(primitives_[i]->BoundingBox());
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
            centroidBox = centroidBox.Union(primitives_[i]->Center());
        }

        // Decide split axis and split location
        int dim = centroidBox.MaxExtent();
        auto midPoint = (centroidBox.m_max[dim] + centroidBox.m_min[dim]) / 2.f;

        // Partition the primitive array
        auto midIter = std::partition(
            primitives_.begin() + start, primitives_.begin() + start + n,
            [dim, midPoint](const std::shared_ptr<Primitive> &primitive)
            {
                return primitive->Center()[dim] < midPoint;
            });
        auto mid = static_cast<int>(midIter - primitives_.begin());
        
        // Build children nodes
        node->children[0] = build_node(start, mid - start);
        node->children[1] = build_node(mid, n - (mid - start));
        node->box = box;
    }
    
    return node;
}


bool BVHAccel::has_intersection(const Ray &ray) const
{
    BVHNode *nodeStack[64];
    int top = -1;
    nodeStack[++ top] = m_Root;
    std::optional<Intersection> is = {};
    while (top >= 0)
    {
        auto node = nodeStack[top --];

        // Check intersection with node's bounding box
        if (!node->box.ExistIntersection(ray))
        {
            continue;
        }

        if (!node->children[0])
        {
            // Find intersection with leaf node's primitives
            for (int i = node->startIndex; i < node->startIndex + node->primitiveCount; i ++)
            {
                if (primitives_[i]->has_intersection(ray))
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

Bound3f BVHAccel::BoundingBox() const
{
    return m_Root ? m_Root->box : Bound3f();
}

Point3f BVHAccel::Center() const
{
    return BoundingBox().Center();
}
