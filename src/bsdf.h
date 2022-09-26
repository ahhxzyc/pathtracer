#pragma once


#include "Log.h"
#include "common.h"

#include <vector>
#include <memory>

// a BxDF sample, recording sample direction in :
// BxDF: local space
// BSDF: world space
struct BxDFSample
{
    Color3f f;
    float pdf;
    Vec3f wi;
    bool isDelta;
};
enum BxDFType
{
    BSDF_Reflection      = 1 << 0,
    BSDF_Transmission    = 1 << 1,
    BSDF_Delta           = 1 << 2,
    BSDF_All = BSDF_Reflection | BSDF_Transmission | BSDF_Delta
};
inline BxDFType operator|(BxDFType a, BxDFType b)
{
    return static_cast<BxDFType>(int(a) | int(b));
}


// BRDF & BTDF defined in local solid angle space
// therefore all function input and outputs are defined in LOCAL space
class BxDF
{
public:
    BxDF(const Color3f &r) : reflectance(r) {type = BSDF_Reflection;}
    virtual Color3f Eval(const Vec3f &wi) const = 0;
    virtual BxDFSample Sample() const = 0;
    virtual float Pdf(const Vec3f &wi) const = 0;
public:
    Color3f reflectance;
    float samplingWeight;
    BxDFType type;
};

class LambertianDiffuse : public BxDF
{
public:
    LambertianDiffuse(const Color3f &r, const Vec3f &wo) : BxDF(r), wo_(wo) {}
    virtual Color3f Eval(const Vec3f &wi) const override;
    virtual BxDFSample Sample() const override;
    virtual float Pdf(const Vec3f &wi) const override;
private:
    Vec3f wo_;
};

class PhongSpecular : public BxDF
{
public:
    PhongSpecular(const Color3f &r, float ns, const Vec3f &wo)
        : BxDF(r), exponent_(ns), wo_(wo)
    {
    }
    virtual Color3f Eval(const Vec3f &wi) const override;
    virtual BxDFSample Sample() const override;
    virtual float Pdf(const Vec3f &wi) const override;
private:
    float exponent_;
    Vec3f wo_;
};

class BlinnPhongSpecular : public BxDF
{
public:
    BlinnPhongSpecular(const Color3f &r, float ns, const Vec3f &wo)
        : BxDF(r), exponent_(ns), wo_(wo) { }
    virtual Color3f Eval(const Vec3f &wi) const override;
    virtual BxDFSample Sample() const override;
    virtual float Pdf(const Vec3f &wi) const override;
private:
    float exponent_;
    Vec3f wo_;
};

class SpecularReflection : public BxDF
{
public:
    SpecularReflection(const Vec3f &wo) : BxDF(Color3f(1)), wo_(wo) {type = BSDF_Reflection | BSDF_Delta;}
    virtual Color3f Eval(const Vec3f &wi) const override {return Color3f(0); }
    virtual BxDFSample Sample() const override;
    virtual float Pdf(const Vec3f &wi) const override {return 0.f;}
private:
    Vec3f wo_;
};

class SpecularTransmission : public BxDF
{
public:
    SpecularTransmission(const float &ior, const Vec3f &wo, bool backface)
        : BxDF(Color3f(1)), wo_(wo), ior_(ior), backface_(backface)
    { 
        type = BSDF_Transmission | BSDF_Delta; 
    }
    virtual Color3f Eval(const Vec3f &wi) const override { return Color3f(0); }
    virtual BxDFSample Sample() const override;
    virtual float Pdf(const Vec3f &wi) const override { return 0.f; }
private:
    Vec3f wo_;
    float ior_;
    bool backface_;
};

class SpecularReflectionAndTransmission : public BxDF
{
public:
    SpecularReflectionAndTransmission(const float &ior, const Vec3f &wo)
        : BxDF(Color3f(1)), wo_(wo), ior_(ior)
    {
        type = BSDF_Reflection | BSDF_Transmission | BSDF_Delta;
    }
    virtual Color3f Eval(const Vec3f &wi) const override { return Color3f(0); }
    virtual BxDFSample Sample() const override;
    virtual float Pdf(const Vec3f &wi) const override { return 0.f; }
private:
    Vec3f wo_;
    float ior_;
};


class BSDF
{
public:
    Color3f Eval(const Vec3f &wiW) const;
    BxDFSample Sample() const;
    float Pdf(const Vec3f &wiW) const;

    int num_components(BxDFType flags) const;
    void generate_sampling_weights();
    void ensure_conservation();
    void init();
    std::vector<std::shared_ptr<BxDF>> bxdfs;
    CoordinateSystem onb;
};
