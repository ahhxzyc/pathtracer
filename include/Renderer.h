#pragma once

#include "Types.h"
#include "Ray.h"
#include "Scene.h"

#include <vector>
using namespace std;


class Model;

class Renderer {
public:
    Renderer(Scene *pScene);
    ~Renderer() = default;

    // get frame buffer of the rendered scene
    void render();
    // get the color of the traced ray
    Vec3f trace(Ray ray, int depth);
    // save iamge
    void save(const string &filepath) const;
    
    // get diffuse BRDF
    Vec3f brdf(Vec3f wi, Vec3f wo, Intersection inter) const;
    // samplers
    bool brdf_importance_sampling(Vec3f normal, Vec3f wo, float ns, Vec3f &res);
    Intersection sample_light_source(float &pdf);
    Vec3f sample_hemisphere(Vec3f normal, float &pdf) const;

private:
    // the scene
    Scene *m_scene;
    // frame buffer
    vector<Vec3f> fb;
    // samples per pixel
    int m_spp = 10;
    // max depth of recursion
    int max_depth = 3;
};