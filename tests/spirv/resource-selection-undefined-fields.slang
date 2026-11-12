// Check that texture fields that are never written in either arm of a branch do not become
// resource selections. Each arm builds its own struct, so each unwritten field reaches the merge
// as two distinct undefined values. The merged field is undefined on every path, so resource
// selection must not dispatch on it: a call that takes the structs runs once when the written
// fields agree, and once per written texture when they differ, rather than once per combination
// of the unwritten fields.

//TEST:SIMPLE(filecheck=SAME):-target spirv -entry sameTexture -stage compute -validate-ir
//TEST:SIMPLE(filecheck=SAME):-target spirv -entry sameTexture -stage compute -validate-ir -O0
//TEST:SIMPLE(filecheck=CHOSEN):-target spirv -entry chosenTexture -stage compute -validate-ir
//TEST:SIMPLE(filecheck=CHOSEN):-target spirv -entry chosenTexture -stage compute -validate-ir -O0
//TEST:SIMPLE(filecheck=SAME_COMBINED):-target spirv -entry sameCombined -stage compute -validate-ir
//TEST:SIMPLE(filecheck=SAME_COMBINED):-target spirv -entry sameCombined -stage compute -validate-ir -O0
//TEST:SIMPLE(filecheck=CHOSEN_COMBINED):-target spirv -entry chosenCombined -stage compute -validate-ir
//TEST:SIMPLE(filecheck=CHOSEN_COMBINED):-target spirv -entry chosenCombined -stage compute -validate-ir -O0

Texture2D<float4> textureA;
Texture2D<float4> textureB;
SamplerState samplerState;
RWStructuredBuffer<float4> outputBuffer;

// Only one of the texture fields is written, and `kind` says which.
struct AnyTexture
{
    Texture2D<float4> texture4;
    Texture2D<float> texture1;
    Texture2D<float2> texture2;
    int kind;
};

AnyTexture makeAnyTexture(Texture2D<float4> texture)
{
    AnyTexture result;
    result.texture4 = texture;
    result.kind = 4;
    return result;
}

float4 sampleAnyTexture(AnyTexture t, float2 uv)
{
    if (t.kind == 1)
        return t.texture1.SampleLevel(samplerState, uv, 0).xxxx;
    if (t.kind == 2)
        return t.texture2.SampleLevel(samplerState, uv, 0).xyxy;
    return t.texture4.SampleLevel(samplerState, uv, 0);
}

[noinline]
float4 sampleBoth(AnyTexture first, AnyTexture second, float2 uv)
{
    return sampleAnyTexture(first, uv) + sampleAnyTexture(second, uv);
}

// Both arms write the same textures, so no choice remains and the call is not duplicated.
//
// SAME: %sameTexture = OpFunction
// SAME-COUNT-1: OpFunctionCall
// SAME-NOT: OpFunctionCall
// SAME: OpFunctionEnd
[numthreads(1, 1, 1)]
void sameTexture(uint3 id: SV_DispatchThreadID)
{
    AnyTexture first;
    AnyTexture second;
    if (id.x != 0)
    {
        first = makeAnyTexture(textureA);
        second = makeAnyTexture(textureB);
    }
    else
    {
        first = makeAnyTexture(textureA);
        second = makeAnyTexture(textureB);
    }
    outputBuffer[0] = sampleBoth(first, second, float2(id.xy));
}

// The arms write different textures to both structs, but under the same condition. Each written
// field is a two-way choice, so the call runs in one of four arms.
//
// CHOSEN: %chosenTexture = OpFunction
// CHOSEN-COUNT-4: OpFunctionCall
// CHOSEN-NOT: OpFunctionCall
// CHOSEN: OpFunctionEnd
[numthreads(1, 1, 1)]
void chosenTexture(uint3 id: SV_DispatchThreadID)
{
    AnyTexture first;
    AnyTexture second;
    if (id.x != 0)
    {
        first = makeAnyTexture(textureA);
        second = makeAnyTexture(textureB);
    }
    else
    {
        first = makeAnyTexture(textureB);
        second = makeAnyTexture(textureA);
    }
    outputBuffer[0] = sampleBoth(first, second, float2(id.xy));
}

// The same choices with combined texture-samplers. An unwritten combined field holds an unwritten
// texture and an unwritten sampler, and neither may become a choice to dispatch on.
Sampler2D<float4> combinedA;
Sampler2D<float4> combinedB;

struct AnyCombined
{
    Sampler2D<float4> combined4;
    Sampler2D<float> combined1;
    Sampler2D<float2> combined2;
    int kind;
};

AnyCombined makeAnyCombined(Sampler2D<float4> combined)
{
    AnyCombined result;
    result.combined4 = combined;
    result.kind = 4;
    return result;
}

float4 sampleAnyCombined(AnyCombined t, float2 uv)
{
    if (t.kind == 1)
        return t.combined1.SampleLevel(uv, 0).xxxx;
    if (t.kind == 2)
        return t.combined2.SampleLevel(uv, 0).xyxy;
    return t.combined4.SampleLevel(uv, 0);
}

[noinline]
float4 sampleBothCombined(AnyCombined first, AnyCombined second, float2 uv)
{
    return sampleAnyCombined(first, uv) + sampleAnyCombined(second, uv);
}

// SAME_COMBINED: %sameCombined = OpFunction
// SAME_COMBINED-COUNT-1: OpFunctionCall
// SAME_COMBINED-NOT: OpFunctionCall
// SAME_COMBINED: OpFunctionEnd
[numthreads(1, 1, 1)]
void sameCombined(uint3 id: SV_DispatchThreadID)
{
    AnyCombined first;
    AnyCombined second;
    if (id.x != 0)
    {
        first = makeAnyCombined(combinedA);
        second = makeAnyCombined(combinedB);
    }
    else
    {
        first = makeAnyCombined(combinedA);
        second = makeAnyCombined(combinedB);
    }
    outputBuffer[0] = sampleBothCombined(first, second, float2(id.xy));
}

// CHOSEN_COMBINED: %chosenCombined = OpFunction
// CHOSEN_COMBINED-COUNT-4: OpFunctionCall
// CHOSEN_COMBINED-NOT: OpFunctionCall
// CHOSEN_COMBINED: OpFunctionEnd
[numthreads(1, 1, 1)]
void chosenCombined(uint3 id: SV_DispatchThreadID)
{
    AnyCombined first;
    AnyCombined second;
    if (id.x != 0)
    {
        first = makeAnyCombined(combinedA);
        second = makeAnyCombined(combinedB);
    }
    else
    {
        first = makeAnyCombined(combinedB);
        second = makeAnyCombined(combinedA);
    }
    outputBuffer[0] = sampleBothCombined(first, second, float2(id.xy));
}
