// A combined texture-sampler handle (`Sampler2D.Handle`) carried in an interface value is packed
// into 16 bytes of the AnyValue, a texture ID and a sampler ID, and rebuilt from them (#13823). This
// checks that the rebuilt handle still samples the bound texture.
// descriptor-handle-combined-sampler-dynamic-dispatch.slang checks the Metal output.
//
// The Metal back end of the test harness has no combined handle to bind, so the shader makes one
// from a texture handle and a sampler handle. On Metal a combined handle is laid out as the texture
// ID followed by the sampler ID, so the two 8-byte IDs are put together with `reinterpret`. That
// `reinterpret` goes through the same AnyValue packing as the interface value.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-mtl -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=MSL): -target metal -entry computeMain -stage compute

interface IDataSource
{
    float getValue(float2 uv);
}

struct TextureSource : IDataSource
{
    Sampler2D.Handle tex;
    float offset;

    float getValue(float2 uv) { return tex.SampleLevel(uv, 0).x + offset; }
}

struct ConstSource : IDataSource
{
    float value;

    float getValue(float2 uv) { return value; }
}

IDataSource makeSource(int kind, Sampler2D.Handle tex)
{
    if (kind == 0)
    {
        TextureSource ts;
        ts.tex = tex;
        ts.offset = 100.0;
        return ts;
    }
    ConstSource cs;
    cs.value = 42.0;
    return cs;
}

Sampler2D.Handle makeCombinedHandle(Texture2D.Handle t, SamplerState.Handle s)
{
    return reinterpret<Sampler2D.Handle>(uint4(reinterpret<uint2>(t), reinterpret<uint2>(s)));
}

//TEST_INPUT: Texture2D(size=4, content = one):name=tex
uniform Texture2D.Handle tex;

//TEST_INPUT: Sampler:name=samp
uniform SamplerState.Handle samp;

//TEST_INPUT:ubuffer(data=[0 1], stride=4):name=kindBuffer
StructuredBuffer<int> kindBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Sampler2D.Handle combined = makeCombinedHandle(tex, samp);

    // Sampled directly, without an interface value.
    // CHECK: 1.0
    outputBuffer[0] = combined.SampleLevel(float2(0.5), 0).x;

    // The kind comes from a buffer, so the implementation is chosen at run time and the handle
    // goes through the AnyValue.
    IDataSource texSrc = makeSource(kindBuffer[0], combined);
    // CHECK: 101.0
    outputBuffer[1] = texSrc.getValue(float2(0.5));

    IDataSource constSrc = makeSource(kindBuffer[1], combined);
    // CHECK: 42.0
    outputBuffer[2] = constSrc.getValue(float2(0.5));
}

// The texture and sampler IDs are packed into a uint4 and the combined handle is rebuilt from it,
// texture first.
// MSL: Sampler2D_[[#]] {{.*}} = { _slang_reinterpretBits<texture2d<float, access::sample>>(ulong({{.*}}field0_[[#]]) | (ulong({{.*}}field1_[[#]]) << 32U)), _slang_reinterpretBits<sampler>(ulong({{.*}}field2_[[#]]) | (ulong({{.*}}field3_[[#]]) << 32U)) };
// MSL-NOT: (ulong)(
