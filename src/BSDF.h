#pragma once

#include "Types.h"
#include "Log.h"

#include <vector>
#include <memory>

struct BxDFSample
{
    // Local sampled direction
    Vec3f direction;
    float pdf;
};

class BxDF
{
public:
    virtual Color3f Eval(const Vec3f &wi, const Vec3f &wo) const = 0;
    virtual BxDFSample Sample() const = 0;
};

class DiffuseBRDF : public BxDF
{
public:
    DiffuseBRDF(const Color3f &albedo) : m_Albedo(albedo) {}

    virtual Color3f Eval(const Vec3f &wi, const Vec3f &wo) const override
    {
        return m_Albedo / PI;
    }

    virtual BxDFSample Sample() const override
    {
        LOG_ERROR("Not implemented");
        return {};
    }

private:
    Color3f m_Albedo;
};

class BSDF
{
public:
    Color3f Eval(const Vec3f &wi, const Vec3f &wo) const
    {
        Color3f ret(0.f);
        for (auto &bxdf : bxdfs)
        {
            ret += bxdf->Eval(wi, wo);
        }
        return ret;
    }
    std::vector<std::shared_ptr<BxDF>> bxdfs;
};
