#include "bsdf.h"

static const auto s_LocalN = Vec3f(0,0,1);

Color3f LambertianDiffuse::Eval(const Vec3f &wi) const
{
    auto NoL = glm::dot(s_LocalN, wi);
    return NoL > 0.f ? reflectance_ / PI : Color3f(0);
}

BxDFSample LambertianDiffuse::Sample() const
{
    // cosine hemisphere sampling
    float phi = rand01() * 2 * PI;
    float theta = 0.5f * acos(1 - 2 * rand01());
    Vec3f v(
        sin(theta) * cos(phi),
        sin(theta) * sin(phi),
        cos(theta)
    );
    auto pdf = std::abs(v[2]) / PI;
    return { pdf, v };
}

float LambertianDiffuse::Pdf(const Vec3f &wi) const
{
    auto NoL = glm::dot(s_LocalN, wi);
    return NoL > 0.f ? NoL / PI : 0.f;
}

Color3f BlinnPhongSpecular::Eval(const Vec3f &wi) const
{
    auto H = glm::normalize(wi + m_Wo);
    auto NoH = abs_dot(H, s_LocalN);
    return reflectance_ * (m_Shininess + 2) / (2.f * PI) * std::pow(NoH, m_Shininess);
}

BxDFSample BlinnPhongSpecular::Sample() const
{
    // random halfway vector
    auto u = rand01(), v = rand01();
    auto phi = 2 * PI * u;
    auto cosTheta = std::pow(v, 1.f / (m_Shininess + 1));
    auto sinTheta = std::sqrt(1.f - cosTheta * cosTheta);
    Vec3f H(
        sinTheta * cos(phi),
        sinTheta * sin(phi),
        cosTheta
    );
    auto wi = -m_Wo + H * 2.f * glm::dot(m_Wo, H);
    auto pdf = (m_Shininess + 1) / (2 * PI) * std::pow(cosTheta, m_Shininess);
    return {pdf, wi};
}

float BlinnPhongSpecular::Pdf(const Vec3f &wi) const
{
    auto H = glm::normalize(wi + m_Wo);
    auto NoH = abs_dot(H, s_LocalN);
    return (m_Shininess + 1) / (2 * PI) * std::pow(NoH, m_Shininess);
}

Color3f BSDF::Eval(const Vec3f &wiW) const
{
    auto wi = onb.ToLocal(wiW);
    Color3f ret(0.f);
    for (auto &bxdf : bxdfs)
    {
        ret += bxdf->Eval(wi);
    }
    return ret;
}

BxDFSample BSDF::Sample() const
{
    // this method uses a uniform strategy to facilitate
    // multiple components of the BSDF

    // randomly choose one BxDF to sample
    int n = bxdfs.size();
    auto index = std::min(static_cast<int>(rand01() * n), n-1);
    auto sample = bxdfs[index]->Sample();
    sample.wi = onb.ToWorld(sample.wi);
    sample.pdf = Pdf(sample.wi);
    return sample;
}

float BSDF::Pdf(const Vec3f &wiW) const
{
    auto wi = onb.ToLocal(wiW);

    // the average of all pdfs
    float ret = 0.f;
    for (auto bxdf : bxdfs)
        ret += bxdf->Pdf(wi);
    ret /= bxdfs.size();
    return ret;
}

void BSDF::unify_reflectance()
{
    for (auto bxdf : bxdfs)
    {
        bxdf->reflectance_ = Color3f(1, 1, 1);
    }
}
