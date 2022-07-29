#pragma once


#include <vector>
#include "Triangle.h"
#include "OctTree.h"
#include "Skybox.h"
using namespace std;

class Scene {
public:
    Scene(int w, int h);
    ~Scene();

    void init_octtree();

    // load a model from file
    void addModel(const string &filepath);
    // load a skybox
    void addSkybox(const string &filepath);
    // find ray intersection
    void intersect(Ray ray, Intersection &inter);
    // get a ray into (x.y)
    Ray getRay(int x, int y);

public:
    Vec3f m_up = Vec3f(0,1,0);
    Vec3f m_eye = Vec3f(0,1,6.8);
    Vec3f m_lookat = Vec3f(0,1,5.8);
    float m_fov = 19.5;

    Vec3f m_front;
    Vec3f m_right;
    // width and height of the scene (in pixels)
    int m_width;
    int m_height;
    // triangles and lights in the scene
    vector<Triangle> m_tris;
    vector<int> m_light_ids;
    // octtree
    OctTree *m_octtree = nullptr;
    // skybox
    Skybox *m_skybox = nullptr;
};
