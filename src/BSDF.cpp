#include "bsdf.h"

float localCos(const Vec3f &v)
{
    return v.z;
}
Vec3f localReflect(const Vec3f &v)
{
    return Vec3f(-v.x, -v.y, v.z);
}
Vec3f reflect(const Vec3f &v, const Vec3f &normal)
{
    return -v + normal * 2.f * glm::dot(normal, v);
}

Color3f LambertianDiffuse::Eval(const Vec3f &wi) const
{
    auto NoL = localCos(wi);
    return NoL > 0.f ? reflectance / PI : Color3f(0);
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
    auto NoL = localCos(wi);
    return NoL > 0.f ? NoL / PI : 0.f;
}

Color3f BlinnPhongSpecular::Eval(const Vec3f &wi) const
{
    float NoV = localCos(wo_);
    float NoL = localCos(wi);
    if (NoV < 0.f || NoL < 0.f)
        return Color3f(0);
    auto H = glm::normalize(wi + wo_);
    auto NoH = localCos(H);
    //float normFactor = (exponent_ + 2) * (exponent_ + 4) / (8.f * PI * (std::pow(2, -exponent_/2.f) + exponent_));
    float normFactor = (exponent_ + 2) / (2.f * PI);
    return reflectance * normFactor * std::pow(NoH, exponent_);
}

BxDFSample BlinnPhongSpecular::Sample() const
{
    // random halfway vector
    auto u = rand01(), v = rand01();
    auto phi = 2 * PI * u;
    auto cosTheta = std::pow(v, 1.f / (exponent_ + 1));
    auto sinTheta = std::sqrt(1.f - cosTheta * cosTheta);
    Vec3f H(
        sinTheta * cos(phi),
        sinTheta * sin(phi),
        cosTheta
    );
    auto wi = reflect(wo_, H);
    if (localCos(wi) < 0.f)
    {
        return { 0.f, Vec3f(0.f) };
    }
    // TODO: pdf normalization
    auto pdf = (exponent_ + 1) / (2.f * PI) * std::pow(cosTheta, exponent_);
    return {pdf, wi};
}

float BlinnPhongSpecular::Pdf(const Vec3f &wi) const
{
    float NoV = localCos(wo_);
    float NoL = localCos(wi);
    if (NoV < 0.f || NoL < 0.f)
        return 0.f;
    auto H = glm::normalize(wi + wo_);
    auto NoH = localCos(H);
    // TODO: pdf normalization
    return (exponent_ + 1) / (2.f * PI) * std::pow(NoH, exponent_);
}

Color3f BSDF::Eval(const Vec3f &wiW) const
{
    auto wi = onb.ToLocal(wiW);
    Color3f ret(0.f);
    for (auto &bxdf : bxdfs)
    {
        ret += bxdf->Eval(wi);
    }
    //if (glm::length(ret) > 2.f)
    //{
    //    LOG_INFO("{},{},{}", ret.x, ret.y, ret.z);
    //}
    return ret;
}

BxDFSample BSDF::Sample() const
{
    // choose bxdf by their sampling weights
    std::vector<float> weightPrefixSum(bxdfs.size());
    for (int i = 0; i < bxdfs.size(); i ++)
    {
        weightPrefixSum[i] = bxdfs[i]->samplingWeight;
        if (i)
            weightPrefixSum[i] += weightPrefixSum[i-1];
    }
    auto it = std::lower_bound(weightPrefixSum.begin(), weightPrefixSum.end(), rand01() * weightPrefixSum.back());
    int index = std::min(int(it - weightPrefixSum.begin()), (int)weightPrefixSum.size()-1);

    // sample
    auto sample = bxdfs[index]->Sample();
    sample.wi = onb.ToWorld(sample.wi);
    sample.pdf = Pdf(sample.wi);
    return sample;

    //int n = bxdfs.size();
    //auto index = std::min(static_cast<int>(rand01() * n), n-1);
    //auto sample = bxdfs[index]->Sample();
    //sample.wi = onb.ToWorld(sample.wi);
    //sample.pdf = Pdf(sample.wi);
    //return sample;
}

float BSDF::Pdf(const Vec3f &wiW) const
{
    auto wi = onb.ToLocal(wiW);

    float ret = 0.f;
    for (auto bxdf : bxdfs)
    {
        ret += bxdf->Pdf(wi) * bxdf->samplingWeight;
    }
    return ret;
}

void BSDF::generate_sampling_weights()
{
    auto luminance = [](const Color3f &r)
    {
        return r.x * 0.212671f + r.y * 0.715160f + r.z * 0.072169f;
    };
    std::vector<float> lums(bxdfs.size());
    float luminanceSum = 0.f;
    for (int i = 0; i < bxdfs.size(); i ++)
    {
        lums[i] = luminance(bxdfs[i]->reflectance);
        luminanceSum += lums[i];
    }

    if (luminanceSum == 0)
        return;
    float inverseLumSum = 1.f / luminanceSum;
    for (int i = 0; i < bxdfs.size(); i ++)
    {
        bxdfs[i]->samplingWeight = lums[i] * inverseLumSum;
    }
}

void BSDF::unify_reflectance()
{
    //for (auto bxdf : bxdfs)
    //{
    //    bxdf->reflectance = Color3f(1, 1, 1);
    //}
    Color3f reflectanceSum(0.f);
    for (auto bxdf : bxdfs)
    {
        reflectanceSum += bxdf->reflectance;
    }
    float maxComponent = std::max(reflectanceSum.x, std::max(reflectanceSum.y, reflectanceSum.z));
    if (maxComponent < 1.f)
        return;
    for (auto bxdf : bxdfs)
    {
        bxdf->reflectance /= maxComponent;
    }
}

Color3f PhongSpecular::Eval(const Vec3f &wi) const
{
    float NoV = localCos(wo_);
    float NoL = localCos(wi);
    if (NoV < 0.f || NoL < 0.f)
        return Color3f(0);
    auto R = localReflect(wo_);
    auto LoR = abs_dot(wi, R);
    return reflectance * (exponent_ + 2) / (2.f * PI) * std::pow(LoR, exponent_);
}

BxDFSample PhongSpecular::Sample() const
{
    // random vector in reflect space
    auto u = rand01(), v = rand01();
    auto phi = 2 * PI * u;
    auto cosTheta = std::pow(v, 1.f / (exponent_ + 1));
    auto sinTheta = std::sqrt(1.f - cosTheta * cosTheta);
    Vec3f vec(
        sinTheta * cos(phi),
        sinTheta * sin(phi),
        cosTheta
    );
    // transform to normal local space
    auto R = localReflect(wo_);
    auto wi = CoordinateSystem(R).ToWorld(vec);
    if (localCos(wi) < 0.f)
    {
        return {0.f, Vec3f(0.f)};
    }
    auto pdf = (exponent_ + 1) / (2 * PI) * std::pow(cosTheta, exponent_);
    return { pdf, wi };
}

float PhongSpecular::Pdf(const Vec3f &wi) const
{
    float NoV = localCos(wo_);
    float NoL = localCos(wi);
    if (NoV < 0.f || NoL < 0.f)
        return 0.f;
    auto R = localReflect(wo_);
    auto LoR = abs_dot(wi, R);
    return (exponent_ + 1) / (2 * PI) * std::pow(LoR, exponent_);
}
