// Regression test for https://github.com/shader-slang/slang/issues/13823
// On Metal a combined texture-sampler handle (`Sampler2D.Handle`) is a struct of a texture and a
// sampler, two 8-byte resource IDs. Carried in an interface value, it takes 16 bytes of the AnyValue:
// the texture ID in the first two uints and the sampler ID in the next two. Each half is packed and
// unpacked like a single texture or sampler handle, and the struct is rebuilt from the two halves.
// Before the fix the handle got 8 bytes and a `(ulong)` cast of the struct, which is not valid MSL.
// descriptor-handle-combined-sampler-dispatch-runtime.slang checks the round trip at run time.

//TEST:SIMPLE(filecheck=CHECK): -target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=NOCAST): -target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=LIB): -target metallib -entry computeMain -stage compute

interface ILight
{
    float4 eval(float2 uv);
}

struct TexLight : ILight
{
    Sampler2D.Handle tex;
    float4 eval(float2 uv) { return tex.SampleLevel(uv, 0); }
}

// The handle after another field, so it does not start at the first uint of the AnyValue.
struct ScaledTexLight : ILight
{
    float scale;
    Sampler2D.Handle tex;
    float4 eval(float2 uv) { return tex.SampleLevel(uv, 0) * scale; }
}

struct ConstLight : ILight
{
    float4 c;
    float4 eval(float2 uv) { return c; }
}

StructuredBuffer<TexLight> texLights;
StructuredBuffer<ScaledTexLight> scaledLights;
RWStructuredBuffer<float4> outBuf;

ILight makeLight(uint i)
{
    if ((i & 3) == 1)
        return texLights[i];
    if ((i & 3) == 2)
        return scaledLights[i];
    ConstLight c;
    c.c = float4(i);
    return c;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    ILight l = makeLight(tid.x);
    outBuf[tid.x] = l.eval(float2(0.5));
}

// No C-style cast between the handle struct, or either of its halves, and an integer.
// NOCAST-NOT: (ulong)(
// NOCAST-NOT: (Sampler2D_{{[0-9]+}})(
// NOCAST-NOT: (texture2d<{{.*}}>)(
// NOCAST-NOT: (sampler)(

// The combined handle is a texture and a sampler.
// CHECK: struct Sampler2D_[[#]]
// CHECK-NEXT: {
// CHECK-NEXT: texture2d<float, access::sample> texture_[[#]];
// CHECK-NEXT: sampler sampler_[[#]];

// `ScaledTexLight` is the largest type, 24 bytes: `scale`, padding up to the handle's 8-byte
// alignment, and the 16-byte handle.
// CHECK: struct AnyValue24

// TexLight: the texture ID in field0/field1, the sampler ID in field2/field3.
// CHECK-LABEL: packAnyValue24_[[#]](const TexLight_[[#]] thread*
// CHECK: ulong [[TEX0:_S[0-9]+]] = _slang_reinterpretBits<ulong>({{.*}}texture_[[#]])
// CHECK: field0_[[#]] = uint([[TEX0]])
// CHECK: field1_[[#]] = [[TEX0HI:_S[0-9]+]]
// CHECK: ulong [[SMP0:_S[0-9]+]] = _slang_reinterpretBits<ulong>(
// CHECK: field2_[[#]] = uint([[SMP0]])
// CHECK: field3_[[#]] =
// CHECK-LABEL: unpackAnyValue24_[[#]](
// CHECK: Sampler2D_[[#]] [[H0:_S[0-9]+]] = { _slang_reinterpretBits<texture2d<float, access::sample>>(ulong({{.*}}field0_[[#]]) | (ulong({{.*}}field1_[[#]]) << 32U)), _slang_reinterpretBits<sampler>(ulong({{.*}}field2_[[#]]) | (ulong({{.*}}field3_[[#]]) << 32U)) };
// CHECK: tex_[[#]] = [[H0]];

// ScaledTexLight: `scale` in field0, then the texture ID in field1/field2 and the sampler ID in
// field3/field4.
// CHECK-LABEL: packAnyValue24_[[#]](const ScaledTexLight_[[#]] thread*
// CHECK: field0_[[#]] = as_type<uint>({{.*}}scale_[[#]])
// CHECK: ulong [[TEX1:_S[0-9]+]] = _slang_reinterpretBits<ulong>({{.*}}texture_[[#]])
// CHECK: field1_[[#]] = uint([[TEX1]])
// CHECK: field2_[[#]] =
// CHECK: ulong [[SMP1:_S[0-9]+]] = _slang_reinterpretBits<ulong>(
// CHECK: field3_[[#]] = uint([[SMP1]])
// CHECK: field4_[[#]] =
// CHECK-LABEL: unpackAnyValue24_[[#]](
// CHECK: scale_[[#]] = as_type<float>({{.*}}field0_[[#]])
// CHECK: Sampler2D_[[#]] [[H1:_S[0-9]+]] = { _slang_reinterpretBits<texture2d<float, access::sample>>(ulong({{.*}}field1_[[#]]) | (ulong({{.*}}field2_[[#]]) << 32U)), _slang_reinterpretBits<sampler>(ulong({{.*}}field3_[[#]]) | (ulong({{.*}}field4_[[#]]) << 32U)) };
// CHECK: tex_[[#]] = [[H1]];

// LIB: define void @computeMain
