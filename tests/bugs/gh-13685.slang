// Regression test for https://github.com/shader-slang/slang/issues/13685
//
// Two never-written texture values that meet at a control-flow merge must not
// force a texture-typed local (E56004 on Khronos targets, E56019 on Metal). The
// merged value is undefined, so SPIR-V should use an `OpUndef` of the image type.

//TEST:SIMPLE(filecheck=SPIRV):-target spirv -entry main -stage fragment
//TEST:SIMPLE(filecheck=SPIRV):-target spirv -entry main -stage fragment -g
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage fragment
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage fragment -g
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage fragment
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage fragment -g
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage fragment
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage fragment -g
//TEST:SIMPLE(filecheck=MINOPT):-target spirv -entry main -stage fragment -minimum-slang-optimization -DNO_LOOP
//TEST:SIMPLE(filecheck=MINOPT):-target spirv -entry main -stage fragment -minimum-slang-optimization -DNO_LOOP -g

// SPIRV-NOT: {{E56004|E56019}}
// SPIRV: OpEntryPoint
// SPIRV: %[[IMG:[A-Za-z0-9_]+]] = OpTypeImage
// SPIRV-NOT: OpTypePointer Function %[[IMG]]
// SPIRV: OpUndef %[[IMG]]

// With -minimum-slang-optimization, peephole forwards a block-local value only after a
// dominance check, which that mode skips. The loop case keeps its texture param there,
// so this line covers the merges outside a loop.
// MINOPT: %[[IMG:[A-Za-z0-9_]+]] = OpTypeImage
// MINOPT-NOT: OpTypePointer Function %[[IMG]]
// MINOPT: OpUndef %[[IMG]]

// The GLSL and WGSL lines only guard against E56004/E56019 coming back. The code
// emitted for a lone undefined texture is still rejected by glslang and Tint, with
// or without a merge.
// GLSL-NOT: {{E56004|E56019}}
// GLSL: void main()

// WGSL-NOT: {{E56004|E56019}}
// WGSL: @fragment

// METAL-NOT: {{E56004|E56019}}
// METAL: fragment

struct OptionalTexture
{
    bool enabled;
    Texture2D<float4> texture;
};

SamplerState samplerState;
cbuffer Settings
{
    bool replaceValue;
    bool sampleLocal;
    int loopCount;
};

float4 readTexture(OptionalTexture value, float2 uv)
{
    if (value.enabled)
        return value.texture.SampleLevel(samplerState, uv, 0);
    return float4(0, 0, 0, 1);
}

// The same merge without a struct: two never-written texture locals meet at the
// end of the `if`.
float4 readLocal(float2 uv)
{
    Texture2D<float4> t;
    if (replaceValue)
    {
        Texture2D<float4> r;
        t = r;
    }
    if (sampleLocal)
        return t.SampleLevel(samplerState, uv, 0);
    return float4(0, 0, 0, 1);
}

// A merge of a never-written texture with a real one, where the never-written texture is also
// sampled on the other path. The merge folds to the real texture, and the never-written one is
// refined to it.
Texture2D<float4> realTexture;
float4 readMixed(float2 uv)
{
    Texture2D<float4> t;
    float4 r = 0;
    if (replaceValue)
        t = realTexture;
    else
        r = t.SampleLevel(samplerState, uv, 0);
    return r + t.SampleLevel(samplerState, uv, 0);
}

#ifndef NO_LOOP
// The same merge inside a loop. Once the inner merge is forwarded, the loop-header
// param's incoming values are an undefined value and the param itself.
float4 readInLoop(float2 uv)
{
    Texture2D<float4> t;
    for (int i = 0; i < loopCount; i++)
    {
        if (replaceValue)
        {
            Texture2D<float4> r;
            t = r;
        }
    }
    if (sampleLocal)
        return t.SampleLevel(samplerState, uv, 0);
    return float4(0, 0, 0, 1);
}
#endif

[shader("fragment")]
float4 main(float2 uv: TEXCOORD) : SV_Target
{
    OptionalTexture value;
    value.enabled = false;
    if (replaceValue)
    {
        OptionalTexture replacement;
        replacement.enabled = false;
        value = replacement;
    }
    float4 result = readTexture(value, uv) + readLocal(uv) + readMixed(uv);
#ifndef NO_LOOP
    result += readInLoop(uv);
#endif
    return result;
}
