#pragma once

#include "Types.h"
#include "Ray.h"
#include "Scene.h"

#include <vector>
#include <optional>
#include <tuple>
using namespace std;


class Model;

class Renderer {
public:
    Renderer(Scene *pScene);
    ~Renderer() = default;

    // get frame buffer of the rendered scene
    void render();
    // get the color of the traced ray
    Vec3f trace01(Ray ray, int depth);
    Vec3f trace_balanced(Ray ray, int depth);
    // save iamge
    void save(const string &filepath) const;
    
    // samplers
    bool brdf_importance_sampling(Vec3f normal, Vec3f wo, float ns, Vec3f &res);
    std::tuple<Vec3f,float> sample_light_source();
    Intersection sample_light_source_uniform(float &pdf);
    std::optional<float> light_pdf(const Ray& ray);
    float cos_hemisphere_pdf(const Vec3f& dir, const Vec3f &normal);
    Vec3f sample_hemisphere(Vec3f normal, float &pdf) const;

private:
    // the scene
    Scene *m_scene;
    // frame buffer
    vector<Vec3f> fb;
    // samples per pixel
    int m_spp = 5000;
    // max depth of recursion
    int m_max_depth = 6;
};