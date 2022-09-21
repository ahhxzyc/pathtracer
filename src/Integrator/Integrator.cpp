#include "integrator/Integrator.h"
#include "Scene.h"
#include "Intersection.h"
#include "Log.h"
#include "Utils.h"

Integrator::Integrator(const std::shared_ptr<Camera> &camera) : m_Camera(camera)
{

}

void Integrator::Render(const Scene &scene)
{
    auto size = m_Camera->film->Size();
#if !defined(DEBUG) & !defined(_DEBUG)
#pragma omp parallel for
#endif
    for (int pix = 0; pix < size.x * size.y; pix ++)
    {
        auto wi = pix % size.x, hi = pix / size.x;
        auto color = Li(m_Camera->GetRay({ wi, hi }), scene, 6);
        m_Camera->film->AddSample({ wi, hi }, color);
    }
}

Color3f AllLightsIntegralEstimator(const Scene& scene, Intersection &is, const Vec3f& wo)
{
    Color3f L(0.f);
    for (auto &light : scene.m_Lights)
    {
        L += OneLightIntegralEstimator(scene, is, wo, *light);
    }
    return L;
}

Color3f RandomLightIntegralEstimator(const Scene &scene, Intersection &is, const Vec3f& wo)
{
    //auto mat = is.primitive->GetMaterial();
    //if (glm::length2(mat->ks) < 0.01f)
    {
        auto u = rand01();
        int n = scene.m_Lights.size();
        auto index = std::min(static_cast<int>(u * n), n - 1);
        auto invPdf = static_cast<float>(n);
        return OneLightIntegralEstimator(scene, is, wo, *scene.m_Lights[index]) * invPdf;
    }

    //// assign higher priority to lights with 
    //auto reflectDir = -is.wo + is.normal * 2.f * glm::dot(is.normal, is.wo);
    //std::vector<float> weights(scene.m_Lights.size());
    //for (int i = 0; i < scene.m_Lights.size(); i ++)
    //{
    //    auto lightDir = glm::normalize(scene.m_Lights[i]->Center() - is.point);
    //    float LoR = glm::dot(lightDir, reflectDir);
    //    auto weight = std::pow(LoR, 10.f);
    //    //float weight = 1.0f;
    //    weights[i] = weight;
    //    if (i)
    //        weights[i] += weights[i-1];
    //}

    //// pick a light
    //auto randomWeight = rand01() * weights.back();
    //auto it = std::lower_bound(weights.begin(), weights.end(), randomWeight);
    //if (it == weights.end())
    //    it -- ;
    //int index = it - weights.begin();
    //auto invPdf = weights.back() / (index > 0 ? weights[index] - weights[index - 1] : weights[index]);
    //return OneLightIntegralEstimator(scene, is, wo, *scene.m_Lights[index]) * invPdf;
}

Color3f OneLightIntegralEstimator(const Scene &scene, Intersection &is, const Vec3f &wo, const Light& light)
{
    Color3f L(0.f);
    
    float lightWeight = 0.f;
    float bsdfWeight = 0.f;

    // sample light
    {
        auto sample = light.Sample(is.point);
        float tmax = glm::length(sample.point - is.point) - 0.0001f;
        if (!scene.GetAggregate().ExistIntersection({ is.point, sample.wi }, 0.001f, tmax))
        {
            auto transport = is.bsdf.Eval(sample.wi) * abs_dot(is.normal, sample.wi);
            auto weight = BalanceHeuristic(sample.pdf, is.bsdf.Pdf(sample.wi));
            //auto weight = 1.f;
            L += transport * sample.Le * weight / sample.pdf;

            lightWeight = weight;
        }
    }

    // sample BSDF
    {
        auto sample = is.bsdf.Sample();
        auto lightPdf = light.Pdf(is, sample.wi);
        Ray ray(is.point, sample.wi);
        auto lightIs = scene.GetAggregate().Intersect(ray, 0.001f, std::numeric_limits<float>::max());
        if (lightIs && !lightIs->backface && lightIs->primitive->GetAreaLight() == &light)
        {
            auto transport = is.bsdf.Eval(sample.wi) * abs_dot(is.normal, sample.wi);
            auto weight = BalanceHeuristic(sample.pdf, lightPdf);
            L += transport * light.Radiance(ray) * weight / sample.pdf;

            bsdfWeight = weight;
        }
    }

    //auto sum = lightWeight + bsdfWeight;
    //if (sum > 0.001f)
    //{
    //    L = Color3f(1,0,0) + lightWeight/sum*Color3f(-1,1,0);
    //}

    return L;
}

Color3f LightIntegralEstimator(const Scene &scene, Intersection &is, const Vec3f &wo)
{
    Color3f L(0.f);

    float lightWeight = 0.f;
    float bsdfWeight = 0.f;

    // sample from lights
    {
        // uniformly choose a light
        auto u = rand01();
        int n = scene.m_Lights.size();
        auto index = std::min(static_cast<int>(u * n), n - 1);
        auto invPdf = static_cast<float>(n);
        auto &light = *scene.m_Lights[index];

        // sample the light
        auto sample = light.Sample(is.point);
        float tmax = glm::length(sample.point - is.point) - 0.0001f;
        if (!scene.GetAggregate().ExistIntersection({ is.point, sample.wi }, 0.001f, tmax))
        {
            auto transport = is.bsdf.Eval(sample.wi) * abs_dot(is.normal, sample.wi);
            auto weight = BalanceHeuristic(sample.pdf, is.bsdf.Pdf(sample.wi));
            L += transport * sample.Le * weight / sample.pdf * invPdf;

            lightWeight = weight;
        }
    }

    // sample BSDF
    {
        // shoot a ray into the scene
        auto sample = is.bsdf.Sample();
        Ray ray(is.point, sample.wi);
        auto lightIs = scene.GetAggregate().Intersect(ray, 0.001f, std::numeric_limits<float>::max());
        if (lightIs && !lightIs->backface && lightIs->primitive->GetAreaLight())
        {
            auto &light = *lightIs->primitive->GetAreaLight();
            auto transport = is.bsdf.Eval(sample.wi) * abs_dot(is.normal, sample.wi);
            auto lightPdf = light.Pdf(is, sample.wi);
            auto weight = BalanceHeuristic(sample.pdf, lightPdf);
            L += transport * light.Radiance(ray) * weight / sample.pdf;

            bsdfWeight = weight;
        }
    }

    return L;
}

float BalanceHeuristic(float pdf1, float pdf2)
{
    auto sum = pdf1 + pdf2;
    return sum > 0.001f ? pdf1 / sum : 0.f;
}
