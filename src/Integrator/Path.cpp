#include "integrator/Path.h"
#include "Utils.h"

//Vec3f sample_hemisphere(Vec3f normal, float &pdf)
//{
//    CoordinateSystem ax(normal);
//    // cosine weighted sampling
//    
//    return ax.ToWorld(v);
//}


PathIntegrator::PathIntegrator(const std::shared_ptr<Camera> &camera)
    :Integrator(camera)
{

}

Color3f PathIntegrator::Li(const Ray &ray, const Scene &scene, int depth /*= 0*/)
{
    Color3f L = Color3f(0.f);

    Color3f beta(1.f);
    auto currentRay = ray;
    for (int bounces = 0; ; bounces ++ )
    {
        auto is = scene.GetAggregate().Intersect(currentRay, 0.001f, std::numeric_limits<float>::max());
        if (!is || is->backface)
            break;
        is->BuildBSDF();

        // Account for emissive surface before first bounce
        auto mat = is->primitive->GetMaterial();
        if (bounces == 0 && mat->IsEmissive())
        {
            L += mat->ke;
        }

        //// direct lighting integral
        //L += beta * RandomLightIntegralEstimator(scene, *is, -ray.dir);

        // Sample direct lighting
        //for (auto &light : scene.m_Lights)
        //{
        //    //auto sample = light->Sample(is->point);
        //    //float tmax = glm::length(sample.point - is->point) - 0.0001f;
        //    //if (!scene.GetAggregate().ExistIntersection({ is->point, sample.wi }, 0.001f, tmax))
        //    //{
        //    //    auto lightBeta = is->bsdf.Eval(sample.wi) * abs_dot(is->normal, sample.wi, 0.f);
        //    //    L += beta * lightBeta * sample.Le / sample.pdf;
        //    //}
        //    L += beta * OneLightIntegralEstimator(scene, *is, -ray.dir, *light);
        //}
        L += beta * LightIntegralEstimator(scene, *is, -ray.dir);

        // Sample for direction of next ray, accumulate path throughput
        auto sample = is->bsdf.Sample();
        currentRay = Ray(is->point, sample.wi);
        beta *= is->bsdf.Eval(sample.wi) * abs_dot(is->normal, sample.wi) / sample.pdf;

        // Russian roulette
        if (bounces > 3)
        {
            auto q = std::min(max(max(beta.x, beta.y), beta.z), 0.95f);
            if (rand01() > q)
                break;
            beta /= q;
        }
    }

    return L;
}
