#include "AABB.h"
#include <limits>

AABB::AABB() : 
        m_min(Vec3f(std::numeric_limits<float>::max(),
                std::numeric_limits<float>::max(),
                std::numeric_limits<float>::max())),
        m_max(Vec3f(std::numeric_limits<float>::min(),
                std::numeric_limits<float>::min(),
                std::numeric_limits<float>::min()))
{
    
}

AABB::AABB(Vec3f minp, Vec3f maxp) :
        m_min(minp), m_max(maxp)
{
    
}

AABB::AABB(Triangle tri) {
    m_min = tri.p[0].cwiseMin(tri.p[1]).cwiseMin(tri.p[2]);
    m_max = tri.p[0].cwiseMax(tri.p[1]).cwiseMax(tri.p[2]);
}

// merge two AABBs
AABB AABB::merge(AABB box) {
    return AABB(
            m_min.cwiseMin(box.m_min),
            m_max.cwiseMax(box.m_max)
    );
}

// intersection with a ray
bool AABB::intersect(Ray ray) {
    Vec3f v0 = (m_min - ray.orig).cwiseQuotient(ray.dir);
    Vec3f v1 = (m_max - ray.orig).cwiseQuotient(ray.dir);
    float t0 = std::numeric_limits<float>::min();
    float t1 = std::numeric_limits<float>::max();
    for (int i = 0; i < 3; i ++ ) {
        t0 = max(t0, min(v0[i], v1[i]));
        t1 = min(t1, max(v0[i], v1[i]));
    }
    return t0 < t1;
}


AABB AABB::getSubBox(int idx) {
    int x = idx & 1;
    int y = (idx & 2) >> 1;
    int z = (idx & 4) >> 2;
    float side_len = (m_max[0] - m_min[0]) * 0.5f;
    Vec3f base(m_min[0] + x * side_len,
                m_min[1] + y * side_len,
                m_min[2] + z * side_len);
    Vec3f temp(base[0] + side_len,
                base[1] + side_len,
                base[2] + side_len);
    return AABB(base, temp);
}


bool AABB::contain(Vec3f p) {
    for (int i = 0; i < 3; i ++ )
        if (p[i] > m_max[i] || p[i] < m_min[i])
            return false;
    return true;
}


bool AABB::contain(Triangle tri) {
    return contain(tri.p[0]) && contain(tri.p[1]) && contain(tri.p[2]);
}