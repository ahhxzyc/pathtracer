#pragma once

#include "Integrator.h"

class WhiteFurnaceIntegrator : public Integrator
{
public:
    WhiteFurnaceIntegrator(const std::shared_ptr<Camera> &camera);
    virtual ~WhiteFurnaceIntegrator() {}

    virtual Color3f Li(const Ray &ray, const Scene &scene, int depth = 0) override;
};