// Regression test for https://github.com/shader-slang/slang/issues/13753
// A texture or sampler DescriptorHandle carried in an interface value is packed into an AnyValue as
// its 64-bit value. Metal spells such a handle as the texture or sampler type itself, which no cast
// converts to or from an integer, so the bits must be read and rebuilt through a `thread` pointer
// instead of a C-style cast.

//TEST:SIMPLE(filecheck=CHECK): -target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=NOCAST): -target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=LIB): -target metallib -entry computeMain -stage compute

interface ILight
{
    float4 eval(float2 uv);
}

struct TexLight : ILight
{
    Texture2D.Handle tex;
    SamplerState.Handle samp;
    float4 eval(float2 uv) { return tex.SampleLevel(samp, uv, 0); }
}

struct ConstLight : ILight
{
    float4 c;
    float4 eval(float2 uv) { return c; }
}

StructuredBuffer<TexLight> texLights;
RWStructuredBuffer<float4> outBuf;

ILight makeLight(uint i)
{
    if ((i & 1) != 0)
        return texLights[i];
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

// No C-style cast between a texture or sampler and an integer anywhere in the output.
// NOCAST-NOT: (ulong)(
// NOCAST-NOT: (texture2d<float, access::sample>)(
// NOCAST-NOT: (sampler)(

// Pack: each handle's bits are read through a `thread ulong` pointer.
// CHECK: texture2d<float, access::sample> _slang_handle_bits_src = {{.*}}tex
// CHECK-NEXT: = *(ulong thread*)(&_slang_handle_bits_src);
// CHECK: sampler _slang_handle_bits_src = {{.*}}samp
// CHECK-NEXT: = *(ulong thread*)(&_slang_handle_bits_src);

// Unpack: each handle is rebuilt from its bits through a pointer to the handle's type.
// CHECK: ulong _slang_handle_bits_src = {{.*}}field0
// CHECK-NEXT: = *(texture2d<float, access::sample> thread*)(&_slang_handle_bits_src);
// CHECK: ulong _slang_handle_bits_src = {{.*}}field2
// CHECK-NEXT: = *(sampler thread*)(&_slang_handle_bits_src);

// LIB: define void @computeMain
