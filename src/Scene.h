#pragma once


#include <vector>
#include "Triangle.h"
#include "OctTree.h"
#include "Skybox.h"
using namespace std;

class Scene {
public:
    Scene();
    ~Scene();

    void init_octtree();

    // load a model from file
    void addModel(const string &filepath);

    // find ray intersection
    Intersection intersect(Ray ray, float tmin = 0.001f, float tmax = 1e10) const;

public:
    // triangles and lights in the scene
    vector<Triangle> m_tris;
    vector<int> m_light_ids;
    // octtree
    OctTree *m_octtree = nullptr;
};
