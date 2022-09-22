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

class WhiteFurnaceIntegrator : public Integrator
{
public:
    WhiteFurnaceIntegrator(const Color3f &rad);
    virtual ~WhiteFurnaceIntegrator() = default;
    virtual Color3f radiance(const Ray &ray, const Scene &scene, int depth = 0) override;
private:
    Color3f incidentRadiance_;
};

float balance_heuristic(float pdf1, float pdf2);
float power_heuristic(float pdf1, float pdf2);