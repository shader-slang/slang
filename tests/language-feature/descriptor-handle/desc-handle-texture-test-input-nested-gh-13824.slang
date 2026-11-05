// A read-only Texture2D.Handle input must get a read-only descriptor (a sampled image on
// Vulkan, an SRV on D3D12) when it is a struct field and when it is carried through an
// interface value. On CUDA a handle has the layout of the resource itself and is bound
// directly, so the cuda lane only confirms the expected values.
//
// The shader has no global uniform data: on D3D12, slang-rhi's root signature puts the
// constant buffers one register too high when global uniforms sit next to ConstantBuffer
// parameters, so the top-level handles are in desc-handle-texture-test-input-gh-13824.slang.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -render-feature bindless -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -output-using-type -profile sm_6_6 -render-feature bindless
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type

// CHECK: type: float

struct S
{
    Texture2D.Handle t;
    SamplerState.Handle samp;
}
//TEST_INPUT: set s = new S{Texture2D(size=4, content = one), Sampler}
ConstantBuffer<S> s;

[anyValueSize(16)]
interface IReader
{
    float read();
}

export struct TextureReader : IReader
{
    Texture2D.Handle t;
    float read() { return t.Load(int3(0)).x; }
}

struct Params
{
    IReader reader;
}
//TEST_INPUT: set gParams = new{ reader: new TextureReader{ Texture2D(size=4, content = one) } }
ConstantBuffer<Params> gParams;
//TEST_INPUT: globalExistentialType __Dynamic

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK-NEXT: 1.0
    outputBuffer[0] = s.t.Load(int3(0)).x;
    // CHECK-NEXT: 1.0
    outputBuffer[1] = s.t.SampleLevel(s.samp, float2(0.5), 0).x;
    // CHECK-NEXT: 1.0
    outputBuffer[2] = gParams.reader.read();
}
