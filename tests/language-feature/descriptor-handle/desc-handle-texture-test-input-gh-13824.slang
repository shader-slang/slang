// A read-only Texture2D.Handle input must use a sampled-image descriptor even when
// its reflection type is uint2, both at the top level and as a struct field. An
// RWTexture2D.Handle input keeps a storage-image descriptor.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -render-feature bindless -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -output-using-type -profile sm_6_6 -render-feature bindless
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type

// CHECK: type: float
// CHECK-NEXT: 1.0
// CHECK-NEXT: 1.0
// CHECK-NEXT: 1.0
// CHECK-NEXT: 2.0
// CHECK-NEXT: 1.0
// CHECK-NEXT: 1.0

//TEST_INPUT: Texture2D(size=4, content = one):name=tex
uniform Texture2D.Handle tex;

//TEST_INPUT: Sampler:name=samp
uniform SamplerState.Handle samp;

//TEST_INPUT: RWTexture2D(format=R32Float, size=4, content = one, mipMaps = 1):name=rwTex
uniform RWTexture2D<float>.Handle rwTex;

struct S
{
    Texture2D.Handle t;
}
//TEST_INPUT: set s = new S{Texture2D(size=4, content = one)}
ConstantBuffer<S> s;

// A directly bound texture is the control: it reads 1 on every lane.
//TEST_INPUT: Texture2D(size=4, content = one):name=boundTex
Texture2D boundTex;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = boundTex.Load(int3(0)).x;
    outputBuffer[1] = tex.Load(int3(0)).x;
    outputBuffer[2] = tex.SampleLevel(samp, float2(0.5), 0).x;
    outputBuffer[3] = rwTex[int2(0, 0)] + 1.0;
    outputBuffer[4] = s.t.Load(int3(0)).x;
    outputBuffer[5] = s.t.SampleLevel(samp, float2(0.5), 0).x;
}
