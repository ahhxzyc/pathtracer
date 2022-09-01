#include "Aggregate.h"


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
