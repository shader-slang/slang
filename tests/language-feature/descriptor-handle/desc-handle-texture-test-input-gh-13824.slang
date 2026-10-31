// A read-only Texture2D.Handle input must get a read-only descriptor (a sampled image on
// Vulkan, an SRV on D3D12), and an RWTexture2D.Handle input keeps a read-write descriptor.
// The struct-field and interface-value placements are in
// desc-handle-texture-test-input-nested-gh-13824.slang. On CUDA a handle has the layout of
// the resource itself and is bound directly, so the cuda lane only confirms the expected
// values.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -render-feature bindless -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -output-using-type -profile sm_6_6 -render-feature bindless
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type

// CHECK: type: float

//TEST_INPUT: Texture2D(size=4, content = one):name=tex
uniform Texture2D.Handle tex;

//TEST_INPUT: Sampler:name=samp
uniform SamplerState.Handle samp;

//TEST_INPUT: RWTexture2D(format=R32Float, size=4, content = one, mipMaps = 1):name=rwTex
uniform RWTexture2D<float>.Handle rwTex;

// A directly bound texture gives the reference value for the handle reads.
//TEST_INPUT: Texture2D(size=4, content = one):name=boundTex
Texture2D boundTex;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK-NEXT: 1.0
    outputBuffer[0] = boundTex.Load(int3(0)).x;
    // CHECK-NEXT: 1.0
    outputBuffer[1] = tex.Load(int3(0)).x;
    // CHECK-NEXT: 1.0
    outputBuffer[2] = tex.SampleLevel(samp, float2(0.5), 0).x;
    // CHECK-NEXT: 1.0
    outputBuffer[3] = rwTex[int2(0, 0)];
}
