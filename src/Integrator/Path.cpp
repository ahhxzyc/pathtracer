#include "Path.h"
#include "Utils.h"

Vec3f sample_hemisphere(Vec3f normal, float &pdf)
{
    // local coordinate
    Vec3f front;
    if (fabs(normal[0]) > fabs(normal[1])) {
        front = glm::normalize(Vec3f(normal[2], 0, -normal[0]));
    } else {
        front = glm::normalize(Vec3f(0, normal[2], -normal[1]));
    }
    Vec3f right = glm::cross(front, normal);
    // cosine weighted sampling
    float phi = rand01() * 2 * PI;
    float theta = 0.5f * acos(1 - 2 * rand01());
    Vec3f v(
        sin(theta) * cos(phi),
        sin(theta) * sin(phi),
        cos(theta)
    );
    Vec3f dir = glm::normalize(v[0]*right + v[1]*front + v[2]*normal);
    pdf = fabs(v[2]) / PI;
    return dir;
}


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
        if (!is)
            break;
        is->BuildBSDF();

        // Account for emissive surface before first bounce
        auto mat = is->primitive->GetMaterial();
        if (bounces == 0 && mat->IsEmissive())
        {
            L += mat->ke;
        }

        // Sample direct lighting
        for (auto &light : scene.lights)
        {
            auto sample = light->Sample(is->point);
            float tmax = glm::length(sample.point - is->point) - 0.0001f;
            if (!scene.GetAggregate().ExistIntersection({ is->point, sample.wi }, 0.001f, tmax))
            {
                auto lightBeta = is->bsdf.Eval({}, {}) * abs_dot(is->normal, sample.wi, 0.f);
                L += beta * lightBeta * sample.Le / sample.pdf;
            }
        }

        // Sample for direction of next ray, accumulate path throughput
        float pdf;
        auto wi = sample_hemisphere(is->normal, pdf);
        currentRay = Ray(is->point, wi);
        beta *= is->bsdf.Eval({},{}) * abs_dot(is->normal, wi) / pdf;

        // Russian roulette
        if (bounces > 3)
        {
            auto q = std::min(max(max(beta.x, beta.y), beta.z), 0.95f);
            //auto q = 0.5f;
            //auto q = std::max(0.05f, 1 - glm::length(beta));
            if (rand01() > q)
                break;
            beta /= q;
        }
    }

    return L;
}
