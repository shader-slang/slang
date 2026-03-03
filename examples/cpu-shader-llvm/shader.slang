struct MyConstantParams
{
    int2 imageSize;
};

RWStructuredBuffer<uint8_t4> outputColors;
ConstantBuffer<MyConstantParams, ScalarDataLayout> constants;

[numthreads(8, 8, 1)]
void renderMandelbrotFractal(uint2 tid : SV_DispatchThreadID, uniform int maxIters)
{
    float2 uv = float2(tid)/float2(constants.imageSize);
    uv = uv * float2(2.47, 2.24) - float2(2.0, 1.12);

    float2 z = float2(0);
    int i = 0;
    while (i < maxIters && dot(z,z) < float(1<<16))
    {
        z = float2(z.x*z.x - z.y*z.y + uv.x, 2*z.x*z.y + uv.y);
        ++i;
    }

    float logZn = log2(dot(z,z)) * 0.5;
    float nu = i < maxIters ? log2(logZn) : 1;
    float t = clamp((i+1-nu)/maxIters, 0.0, 1.0);

    uint8_t value = uint8_t(pow(t, 1.0/2.2) * 255);

    outputColors[tid.x + tid.y * constants.imageSize.x] = uint8_t4(value.xxx, 255);
}
