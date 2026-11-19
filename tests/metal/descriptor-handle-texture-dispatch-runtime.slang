// A texture and a sampler DescriptorHandle carried in an interface value are packed into an AnyValue
// as their 64-bit values and rebuilt from them (#13753). This checks that the rebuilt handles still
// sample the bound texture. descriptor-handle-texture-dynamic-dispatch.slang checks the Metal
// output for each handle kind.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-mtl -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

interface IDataSource
{
    float getValue(float2 uv);
}

struct TextureSource : IDataSource
{
    Texture2D.Handle tex;
    SamplerState.Handle samp;
    float offset;

    float getValue(float2 uv) { return tex.SampleLevel(samp, uv, 0).x + offset; }
}

struct ConstSource : IDataSource
{
    float value;

    float getValue(float2 uv) { return value; }
}

IDataSource makeSource(int kind, Texture2D.Handle tex, SamplerState.Handle samp)
{
    if (kind == 0)
    {
        TextureSource ts;
        ts.tex = tex;
        ts.samp = samp;
        ts.offset = 100.0;
        return ts;
    }
    ConstSource cs;
    cs.value = 42.0;
    return cs;
}

//TEST_INPUT: Texture2D(size=4, content = one):name=tex
uniform Texture2D.Handle tex;

//TEST_INPUT: Sampler:name=samp
uniform SamplerState.Handle samp;

//TEST_INPUT:ubuffer(data=[0 1], stride=4):name=kindBuffer
StructuredBuffer<int> kindBuffer;

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // The kind comes from a buffer, so the implementation is chosen at run time and the handles
    // go through the AnyValue.
    IDataSource texSrc = makeSource(kindBuffer[0], tex, samp);
    // CHECK: 101.0
    outputBuffer[0] = texSrc.getValue(float2(0.5));

    IDataSource constSrc = makeSource(kindBuffer[1], tex, samp);
    // CHECK: 42.0
    outputBuffer[1] = constSrc.getValue(float2(0.5));
}
