// Regression test for #12792: predecessor-constant switch selectors should be
// jump-threaded away. A branch cascade selects a compile-time-constant integer
// tag per path, and a later `switch` consumes it. The tag is constant along
// every incoming edge, so the second dispatch is redundant and should be
// eliminated -- leaving no `OpSwitch` in the emitted SPIR-V, matching the
// control ("fused") form that dispatches directly in each arm.

//TEST:SIMPLE(filecheck=SPLIT): -target spirv-asm -entry splitMain -stage compute -emit-spirv-directly -O3
//TEST:SIMPLE(filecheck=FUSED): -target spirv-asm -entry fusedMain -stage compute -emit-spirv-directly -O3

// The split form must no longer emit a runtime tag dispatch.
// SPLIT-NOT: OpSwitch

// The fused control form never had one.
// FUSED-NOT: OpSwitch

// Also run the threaded (split) path end to end and check it still computes the
// same values as the fused control -- so a rewrite that wires a case to the
// wrong result would fail here even though no OpSwitch survives. The -O3 variant
// runs with the threading pass active (computeMain emits no OpSwitch at -O3).
//TEST:COMPARE_COMPUTE(filecheck-buffer=EQ):-output-using-type -cpu
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=EQ):-cpu -compute -output-using-type -Xslang -O3

struct SampleResult
{
    float3 value;
    float pdf;
    uint tag;
}

interface ILobe
{
    [ForceInline]
    SampleResult sample(float3 wi, float2 u);
}

struct Lobe<let ID : int> : ILobe
{
    float3 tint;
    float roughness;

    [ForceInline]
    SampleResult sample(float3 wi, float2 u)
    {
        const float id = float(ID);
        const float3 axis = normalize(float3(0.2 + 0.03 * id, 0.3 + 0.02 * id, 0.9 - 0.04 * id));
        const float cosine = abs(dot(normalize(wi), axis));
        float response = cosine * (0.8 + 0.03 * id) + roughness * (0.02 + 0.004 * id);
        response = (response * response + u.x * 0.03125) / (1.0 + roughness * (1.0 - cosine));

        SampleResult result;
        result.value = tint * response * (0.75 + 0.25 * u.y);
        result.pdf = saturate(cosine * (0.5 + 0.008 * id) + roughness * 0.125);
        result.tag = ID;
        return result;
    }
}

struct MaterialData
{
    Lobe<0> lobe0; float selection0;
    Lobe<1> lobe1; float selection1;
    Lobe<2> lobe2; float selection2;
    Lobe<3> lobe3; float selection3;
    Lobe<4> lobe4; float selection4;
    Lobe<5> lobe5; float selection5;
    Lobe<6> lobe6; float selection6;
    Lobe<7> lobe7; float selection7;
}

struct InputData
{
    float3 wi;
    float selector;
    float2 u;
    uint materialIndex;
}

[ForceInline]
SampleResult sampleConcrete<T : ILobe>(T lobe, float3 wi, float2 u)
{
    return lobe.sample(wi, u);
}

[ForceInline]
SampleResult finishSample(SampleResult result)
{
    result.value = result.value * 0.875 + result.pdf * float3(0.01, 0.03, 0.04);
    result.pdf = saturate(dot(result.value, float3(0.25, 0.5, 0.25)) + 0.01);
    return result;
}

// Maintainable form: selection and heterogeneous dispatch are separate.
[ForceInline]
SampleResult sampleSplit(MaterialData data, float3 wi, float selector, float2 u)
{
    const float c0 = data.selection0;
    const float c1 = c0 + data.selection1;
    const float c2 = c1 + data.selection2;
    const float c3 = c2 + data.selection3;
    const float c4 = c3 + data.selection4;
    const float c5 = c4 + data.selection5;
    const float c6 = c5 + data.selection6;
    const float c7 = c6 + data.selection7;
    const float target = saturate(selector) * c7;

    int selected;
    if (target < c0) selected = 0;
    else if (target < c1) selected = 1;
    else if (target < c2) selected = 2;
    else if (target < c3) selected = 3;
    else if (target < c4) selected = 4;
    else if (target < c5) selected = 5;
    else if (target < c6) selected = 6;
    else selected = 7;

    SampleResult result = {};
    switch (selected)
    {
    case 0: result = sampleConcrete(data.lobe0, wi, u); break;
    case 1: result = sampleConcrete(data.lobe1, wi, u); break;
    case 2: result = sampleConcrete(data.lobe2, wi, u); break;
    case 3: result = sampleConcrete(data.lobe3, wi, u); break;
    case 4: result = sampleConcrete(data.lobe4, wi, u); break;
    case 5: result = sampleConcrete(data.lobe5, wi, u); break;
    case 6: result = sampleConcrete(data.lobe6, wi, u); break;
    case 7: result = sampleConcrete(data.lobe7, wi, u); break;
    }
    return finishSample(result);
}

// Control form: preserve the selected type until its concrete call.
[ForceInline]
SampleResult sampleFused(MaterialData data, float3 wi, float selector, float2 u)
{
    const float c0 = data.selection0;
    const float c1 = c0 + data.selection1;
    const float c2 = c1 + data.selection2;
    const float c3 = c2 + data.selection3;
    const float c4 = c3 + data.selection4;
    const float c5 = c4 + data.selection5;
    const float c6 = c5 + data.selection6;
    const float c7 = c6 + data.selection7;
    const float target = saturate(selector) * c7;

    SampleResult result;
    if (target < c0) result = sampleConcrete(data.lobe0, wi, u);
    else if (target < c1) result = sampleConcrete(data.lobe1, wi, u);
    else if (target < c2) result = sampleConcrete(data.lobe2, wi, u);
    else if (target < c3) result = sampleConcrete(data.lobe3, wi, u);
    else if (target < c4) result = sampleConcrete(data.lobe4, wi, u);
    else if (target < c5) result = sampleConcrete(data.lobe5, wi, u);
    else if (target < c6) result = sampleConcrete(data.lobe6, wi, u);
    else result = sampleConcrete(data.lobe7, wi, u);
    return finishSample(result);
}

StructuredBuffer<MaterialData> materials;
StructuredBuffer<InputData> inputs;
RWStructuredBuffer<SampleResult> results;

[shader("compute")]
[numthreads(64, 1, 1)]
void splitMain(uint3 tid : SV_DispatchThreadID)
{
    const InputData input = inputs[tid.x];
    results[tid.x] = sampleSplit(materials[input.materialIndex], input.wi, input.selector, input.u);
}

[shader("compute")]
[numthreads(64, 1, 1)]
void fusedMain(uint3 tid : SV_DispatchThreadID)
{
    const InputData input = inputs[tid.x];
    results[tid.x] = sampleFused(materials[input.materialIndex], input.wi, input.selector, input.u);
}

// Build a MaterialData/InputData from a scalar seed so the test needs no complex
// buffer input, then check the threaded split form agrees with the fused control
// component-by-component. Writes 1 on agreement.
MaterialData makeMaterial(float s)
{
    MaterialData d;
    d.lobe0.tint = float3(0.1 + s, 0.2, 0.3); d.lobe0.roughness = 0.4; d.selection0 = 0.1 + 0.05 * s;
    d.lobe1.tint = float3(0.2, 0.3 + s, 0.4); d.lobe1.roughness = 0.3; d.selection1 = 0.1;
    d.lobe2.tint = float3(0.3, 0.4, 0.5 + s); d.lobe2.roughness = 0.2; d.selection2 = 0.1;
    d.lobe3.tint = float3(0.4 + s, 0.5, 0.6); d.lobe3.roughness = 0.1; d.selection3 = 0.1;
    d.lobe4.tint = float3(0.5, 0.6 + s, 0.7); d.lobe4.roughness = 0.5; d.selection4 = 0.1;
    d.lobe5.tint = float3(0.6, 0.7, 0.8 + s); d.lobe5.roughness = 0.6; d.selection5 = 0.1;
    d.lobe6.tint = float3(0.7 + s, 0.8, 0.9); d.lobe6.roughness = 0.7; d.selection6 = 0.1;
    d.lobe7.tint = float3(0.8, 0.9 + s, 1.0); d.lobe7.roughness = 0.8; d.selection7 = 0.1;
    return d;
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=eqBuffer
RWStructuredBuffer<int> eqBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // Sweep the selector across all eight arms so every case is exercised; every
    // per-arm agreement flag is checked below (one ordered EQ line per arm), so a
    // rewrite that mis-wires any single case fails.
    for (int i = 0; i < 8; ++i)
    {
        const float selector = (float(i) + 0.5) / 8.0;
        const MaterialData data = makeMaterial(float(i) * 0.01);
        const float3 wi = float3(0.3, 0.5, 0.8);
        const float2 u = float2(0.25, 0.75);

        SampleResult a = sampleSplit(data, wi, selector, u);
        SampleResult b = sampleFused(data, wi, selector, u);

        float d = abs(a.value.x - b.value.x) + abs(a.value.y - b.value.y) +
                  abs(a.value.z - b.value.z) + abs(a.pdf - b.pdf) + abs(float(a.tag) - float(b.tag));
        eqBuffer[i] = (d < 1e-5) ? 1 : 0;
    }
    // EQ: 1
    // EQ: 1
    // EQ: 1
    // EQ: 1
    // EQ: 1
    // EQ: 1
    // EQ: 1
    // EQ: 1
}
