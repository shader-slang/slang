// Regression test for https://github.com/shader-slang/slang/issues/13823
// On Metal a combined texture-sampler handle (`Sampler2D.Handle`) is 16 bytes, a texture ID and a
// sampler ID. A type holding one does not fit an interface whose AnyValue is 8 bytes, and gets the
// usual diagnostic instead of being packed into 8 bytes.

//TEST:SIMPLE(filecheck=CHECK): -target metal -entry computeMain -stage compute

[anyValueSize(8)]
interface ILight
{
    float4 eval(float2 uv);
}

// CHECK: error[E41019]: type does not fit in size required by interface
// CHECK: type 'TexLight' does not fit in the size required by its conforming interface.
struct TexLight : ILight
{
    Sampler2D.Handle tex;
    float4 eval(float2 uv) { return tex.SampleLevel(uv, 0); }
}

// CHECK: note: sizeof(TexLight) is 16, limit is 8

struct ConstLight : ILight
{
    float2 c;
    float4 eval(float2 uv) { return c.xyxy; }
}

StructuredBuffer<TexLight> texLights;
RWStructuredBuffer<float4> outBuf;

ILight makeLight(uint i)
{
    if ((i & 1) == 1)
        return texLights[i];
    ConstLight c;
    c.c = float2(i);
    return c;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    ILight l = makeLight(tid.x);
    outBuf[tid.x] = l.eval(float2(0.5));
}
