#pragma once

#include <vector>
#include "AABB.h"
#include "Triangle.h"
#include "Intersection.h"
#include "Ray.h"

using namespace std;

struct TreeNode {
    AABB aabb;
    vector<Triangle> tris;
    TreeNode *children[8] = {nullptr};

    // intersection with ray
    void intersect(Ray ray, Intersection &inter, int &cnt, float tmin, float tmax);
};



class OctTree {
public:
    OctTree(const vector<Triangle> &tris);

    // build octtree
    TreeNode *build(AABB bounding, vector<Triangle> tris);
    // intersection with ray
    void intersect(Ray ray, Intersection &inter, int &cnt, float tmin, float tmax);
    
    // int longest_axis(const vector<Triangle> &tris);

    int get_depth(TreeNode *r);

private:
    TreeNode *root = nullptr;
    int m_cnt = 0;
};

