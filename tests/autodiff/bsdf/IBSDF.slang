//TEST_IGNORE_FILE:

static const float M_PI = 3.14159265358979323846; // pi

struct ShadingData
{
    float3 V;
    float3 N;
    float3 T;
    float3 B;

    float3 fromLocal(float3 v)
    {
        return T * v.x + B * v.y + N * v.z;
    }

    float3 toLocal(float3 v)
    {
        return float3(dot(v, T), dot(v, B), dot(v, N));
    }
};

interface IBSDF
{
    float3 eval(const float3 wi, const float3 wo);
}

interface IMaterialInstance
{
    float3 eval(const ShadingData sd, const float3 wo);
}
