#include "bsdf.h"

static BxDFSample sNullSample = {Color3f(0), 0, Vec3f(0), false};

float localCos(const Vec3f &v)
{
    return v.z;
}
float localAbsoluteCos(const Vec3f &v)
{
    return std::abs(v.z);
}
Vec3f localReflect(const Vec3f &v)
{
    return Vec3f(-v.x, -v.y, v.z);
}
Vec3f reflect(const Vec3f &v, const Vec3f &normal)
{
    return -v + normal * 2.f * glm::dot(normal, v);
}
Vec3f faceForward(const Vec3f &n, const Vec3f &v)
{
    return glm::dot(n, v) < 0.f ? -n : n;
}
float fresnel_dielectric(float ior_i, float ior_t, float cos_i, float cos_t)
{
    auto rpara = (ior_t * cos_i - ior_i * cos_t) / (ior_t * cos_i + ior_i * cos_t);
    auto rperp = (ior_i * cos_i - ior_t * cos_t) / (ior_i * cos_i + ior_t * cos_t);
    return (rpara * rpara + rperp * rperp) * 0.5f;
}

Color3f LambertianDiffuse::Eval(const Vec3f &wi) const
{
    auto NoL = localCos(wi), NoV = localCos(wo_);
    if (NoL < 0.f || NoV < 0.f)
        return Color3f(0.f);
    return reflectance / PI;
}

BxDFSample LambertianDiffuse::Sample() const
{
    if (localCos(wo_) < 0.f)
        return sNullSample;

    // cosine hemisphere sampling
    float phi = rand01() * 2 * PI;
    float theta = 0.5f * acos(1 - 2 * rand01());
    Vec3f v( sin(theta) * cos(phi),
             sin(theta) * sin(phi),
             cos(theta));
    auto pdf = std::abs(v[2]) / PI;
    return { Eval(v), pdf, v, false};
}

float LambertianDiffuse::Pdf(const Vec3f &wi) const
{
    auto NoV = localCos(wo_);
    auto NoL = localCos(wi);
    if (NoV < 0.f || NoL < 0.f)
        return 0.f;
    return NoL / PI;
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
    auto NoV = localCos(wo_);
    if (NoV < 0.f)
        return sNullSample;

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
        return sNullSample;
    }
    // TODO: pdf normalization
    auto pdf = (exponent_ + 1) / (2.f * PI) * std::pow(cosTheta, exponent_);
    return {Eval(wi), pdf, wi, false};
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
    
    // accumulate value and pdf
    sample.pdf *= bxdfs[index]->samplingWeight;
    for (int i = 0; i < bxdfs.size(); i ++)
    {
        if (index == i)
            continue;
        sample.f += bxdfs[i]->Eval(sample.wi);
        sample.pdf += bxdfs[i]->Pdf(sample.wi) * bxdfs[i]->samplingWeight;
    }
    sample.wi = onb.ToWorld(sample.wi);
    return sample;
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

int BSDF::num_components(BxDFType flags) const
{
    int ret = 0;
    for (auto &comp : bxdfs)
    {
        if (comp->type & flags)
            ret ++ ;
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
    float sum = 0.f;
    for (int i = 0; i < bxdfs.size(); i ++)
    {
        lums[i] = luminance(bxdfs[i]->reflectance);
        sum += lums[i];
    }

    if (sum == 0)
        return;
    float inverseLumSum = 1.f / sum;
    for (int i = 0; i < bxdfs.size(); i ++)
    {
        bxdfs[i]->samplingWeight = lums[i] * inverseLumSum;
    }
}

void BSDF::ensure_conservation()
{
    // white furnace
    //for (auto comp : bxdfs)
    //    comp->reflectance = Color3f(1);

    Color3f sum(0.f);
    for (auto bxdf : bxdfs)
    {
        sum += bxdf->reflectance;
    }
    float maxComponent = std::max(sum.x, std::max(sum.y, sum.z));
    if (maxComponent < 1.f)
        return;
    for (auto bxdf : bxdfs)
    {
        bxdf->reflectance /= maxComponent;
    }
}

void BSDF::init()
{
    ensure_conservation();
    generate_sampling_weights();
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
    auto NoV = localCos(wo_);
    if (NoV < 0.f)
        return sNullSample;

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
        return sNullSample;
    }
    auto pdf = (exponent_ + 1) / (2 * PI) * std::pow(cosTheta, exponent_);
    return { Eval(wi), pdf, wi, false };
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

BxDFSample SpecularReflection::Sample() const
{
    auto NoV = localCos(wo_);
    if (NoV < 0.f)
        return sNullSample;

    auto R = localReflect(wo_);
    auto f = Color3f(1.f) / localCos(R);
    return {f, 1.f, R, true};
}

BxDFSample SpecularTransmission::Sample() const
{
    float eta_o = 1.f, eta_i = ior_;
    if (backface_)
        std::swap(eta_o, eta_i);
    float cos_o = backface_ ? -localCos(wo_) : localCos(wo_);
    float sin_o = std::sqrt(1.f - cos_o * cos_o);
    float sin_i = eta_o / eta_i * sin_o;
    if (sin_i > 1.f)
    {
        // total internal reflection
        return sNullSample;
    }

    float cos_i = std::sqrt(1.f - sin_i * sin_i);
    auto fresnel = fresnel_dielectric(eta_o, eta_i, cos_o, cos_i);
    auto f = Color3f(1) * eta_o * eta_o / (eta_i * eta_i) * (1.f - fresnel) / cos_i;
    auto wi = - eta_o / eta_i * wo_ + 
        (eta_o / eta_i * cos_o - cos_i) * Vec3f(0,0,1);
    return {f, 1.f, wi, true};
}

BxDFSample SpecularReflectionAndTransmission::Sample() const
{
    auto backface = localCos(wo_) < 0.f;
    float eta_o = 1.f, eta_i = ior_;
    if (backface)
        std::swap(eta_o, eta_i);
    Vec3f n = backface ? Vec3f(0,0,-1) : Vec3f(0,0,1);
    float cos_o = backface ? -localCos(wo_) : localCos(wo_);
    float sin_o = std::sqrt(1.f - cos_o * cos_o);
    float sin_i = eta_o / eta_i * sin_o;
    float cos_i = 0.f;

    // calculate fresnel
    float fresnel = 0.f;
    if (sin_i > 1.f)
        fresnel = 1.f;
    else
    {
        cos_i = std::sqrt(1.f - sin_i*sin_i);
        fresnel = fresnel_dielectric(eta_o, eta_i, cos_o, cos_i);
    }

    BxDFSample sample;
    sample.isDelta = true;
    if (rand01() < fresnel)
    {
        auto wi = localReflect(wo_);
        sample.f = fresnel * Color3f(1.f) / localAbsoluteCos(wi);
        sample.wi = wi;
        sample.pdf = fresnel;
        //LOG_INFO("reflect, fresnel: {}", fresnel);
    }
    else
    {
        auto eta = eta_o / eta_i;
        auto wi = - eta * wo_ + (eta * cos_o - cos_i) * n;
        sample.f = (1 - fresnel) * Color3f(1) * eta * eta / cos_i;
        sample.wi = wi;
        sample.pdf = 1.f - fresnel;
        //LOG_INFO("transmit, cos: {}->{}", cos_o, cos_i);
    }
    return sample;
}
