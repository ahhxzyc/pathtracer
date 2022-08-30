#pragma once

#include "Camera.h"
#include "Ray.h"
//#include "Sampler.h"
#include "Scene.h"

#include <memory>

class Sampler;

class Integrator
{
public:
    Integrator(const std::shared_ptr<Camera>& camera);
    virtual ~Integrator() {}

    void Render(const Scene &scene);
    
    virtual Color3f Li(const Ray& ray, const Scene &scene) = 0;

protected:
    std::shared_ptr<Camera> m_Camera;
    std::shared_ptr<Sampler> m_Sampler;
};