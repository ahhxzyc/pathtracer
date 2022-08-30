#pragma once

#include "Integrator.h"

class WhittedIntegrator : public Integrator
{
public:
    WhittedIntegrator(const std::shared_ptr<Camera> &camera);
    virtual ~WhittedIntegrator() {}

    virtual Color3f Li(const Ray &ray, const Scene &scene) override;
};