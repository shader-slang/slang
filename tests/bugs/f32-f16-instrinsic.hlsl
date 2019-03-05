//TEST:SIMPLE:-profile cs_5_0

struct LightCB
{
    float2 intensity;
    uint intensityPacked;
};

RWStructuredBuffer<LightCB> gLightIn;

[numthreads(1, 1, 1)]
void main()
{
    uint numLights = 0;
    uint stride;
    gLightIn.GetDimensions(numLights, stride);

    gLightIn[0].intensityPacked = f32tof16(gLightIn[0].intensity.x);
    gLightIn[0].intensity.y = f16tof32(gLightIn[0].intensityPacked);
}
