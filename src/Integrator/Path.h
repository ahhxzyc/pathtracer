#pragma once

#include "Integrator.h"

class PathIntegrator : public Integrator
{
public:
    PathIntegrator(const std::shared_ptr<Camera> &camera);
    virtual ~PathIntegrator() {}

    virtual Color3f Li(const Ray &ray, const Scene &scene, int depth = 0) override;
};