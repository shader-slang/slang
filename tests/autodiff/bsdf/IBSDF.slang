//TEST_IGNORE_FILE:

static const float M_PI = 3.14159265358979323846; // pi

interface IBSDF
{
    float3 eval(const float3 wi, const float3 wo);
}
