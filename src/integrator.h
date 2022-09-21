#pragma once

#include "common.h"
#include "camera.h"
#include "scene.h"
#include <memory>

class Integrator
{
public:
    virtual ~Integrator() = default;

    void render(const Scene &scene);
    virtual Color3f radiance(const Ray& ray, const Scene &scene, int depth = 0) = 0;
};

class PathIntegrator : public Integrator
{
public:
    virtual ~PathIntegrator() = default;
    virtual Color3f radiance(const Ray& ray, const Scene &scene, int depth = 0) override;
};


Color3f estimate_direct_all       (const Scene& scene, Intersection &is, const Vec3f& wo);
Color3f estimate_direct_stochastic(const Scene &scene, Intersection &is, const Vec3f& wo);
Color3f estimate_direct_single    (const Scene &scene, Intersection &is, const Vec3f &wo, const Light& light);
Color3f estimate_direct_group_stochastic(const Scene &scene, Intersection &is, const Vec3f &wo);

float balance_heuristic(float pdf1, float pdf2);