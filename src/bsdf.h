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
    float pdf;
    Vec3f wi;
    static BxDFSample Null()
    {
        return {0.f, Vec3f(0,0,1)};
    }
};


// BRDF & BTDF defined in local solid angle space
// therefore all function input and outputs are defined in LOCAL space
class BxDF
{
public:
    BxDF(const Color3f &r) : reflectance(r) {}
    virtual Color3f Eval(const Vec3f &wi) const = 0;
    virtual BxDFSample Sample() const = 0;
    virtual float Pdf(const Vec3f &wi) const = 0;
public:
    Color3f reflectance;
    float samplingWeight;
};

class LambertianDiffuse : public BxDF
{
public:
    LambertianDiffuse(const Color3f &r) : BxDF(r) {}
    virtual Color3f Eval(const Vec3f &wi) const override;
    virtual BxDFSample Sample() const override;
    virtual float Pdf(const Vec3f &wi) const override;
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


class BSDF
{
public:
    Color3f Eval(const Vec3f &wiW) const;
    BxDFSample Sample() const;
    float Pdf(const Vec3f &wiW) const;

    void generate_sampling_weights();
    void unify_reflectance();
    std::vector<std::shared_ptr<BxDF>> bxdfs;
    CoordinateSystem onb;
};
