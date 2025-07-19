//TEST:LANG_SERVER:
// Reproduction of exact original issue structure
module bsdfs;

interface ISampleGenerator
{
    float getValue();
}

struct BSDFContext
{
    float iorI;
    float iorT;
    bool inited;

    __init(float iorI_, float iorT_)
    {
        iorI = iorI_;
        iorT = iorT_;
        inited = true;
    }

    __init()
    {
        iorI = 1.f;
        iorT = 1.f;
        inited = false;
    }
}

interface IBSDF
{
    float eval<S : ISampleGenerator>(float wi, float wo, inout S sg, BSDFContext bc);
}

interface IDifferentiable
{
    // marker interface
}

struct LambertDiffuseBTDF : IBSDF, IDifferentiable
{
    float albedo;
    
    // Private constructor - this causes the issue
    __init(float albedo_)
    {
        this.albedo = albedo_;
    }

    float eval<S : ISampleGenerator>(float wi, float wo, inout S sg, BSDFContext bc)
    {
        return albedo * wo;
    }
}

LambertDiffuseBTDF createTranslucent(float color)
{
//HOVER:54,12
    return LambertDiffuseBTDF(color);
}