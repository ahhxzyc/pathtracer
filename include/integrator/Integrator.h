#pragma once

#include "Camera.h"
#include "Ray.h"
#include "Scene.h"

#include <memory>

class Sampler;

class Integrator
{
public:
    Integrator(const std::shared_ptr<Camera>& camera);
    virtual ~Integrator() {}

    void Render(const Scene &scene);
    
    virtual Color3f Li(const Ray& ray, const Scene &scene, int depth = 0) = 0;

protected:
    std::shared_ptr<Camera> m_Camera;
    std::shared_ptr<Sampler> m_Sampler;
};

Color3f AllLightsIntegralEstimator(const Scene& scene, Intersection &is, const Vec3f& wo);
Color3f RandomLightIntegralEstimator(const Scene &scene, Intersection &is, const Vec3f& wo);
Color3f OneLightIntegralEstimator(const Scene &scene, Intersection &is, const Vec3f &wo, const Light& light);

Color3f LightIntegralEstimator(const Scene &scene, Intersection &is, const Vec3f &wo);

float BalanceHeuristic(float pdf1, float pdf2);