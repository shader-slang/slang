//TEST:SIMPLE(filecheck=VECTOR):-target spirv-asm -entry attributedVector -stage compute -emit-spirv-directly -O0 -validate-ir
//TEST:SIMPLE(filecheck=VECTOR):-target spirv-asm -entry attributedVector -stage compute -emit-spirv-directly -O0 -validate-ir -minimum-slang-optimization
//TEST:SIMPLE(filecheck=AGGREGATE):-target spirv-asm -entry aggregate -stage compute -emit-spirv-directly -O0 -validate-ir
//TEST:SIMPLE(filecheck=AGGREGATE):-target spirv-asm -entry aggregate -stage compute -emit-spirv-directly -O0 -validate-ir -minimum-slang-optimization

// The texture's unorm attribute survives on both loop and conditional-merge parameters.
// It does not change the underlying vector type or prevent direct OpPhi emission.
// Disable downstream optimization so it cannot recreate phis from temporary variables.
// VECTOR: OpPhi %v4float
// VECTOR: OpPhi %v4float

RWTexture2D<unorm float4> tex;
RWStructuredBuffer<float4> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void attributedVector(uint3 tid : SV_DispatchThreadID)
{
    unorm float4 value = tex[uint2(0, 0)];
    for (uint i = 0; i < tid.x; ++i)
    {
        if (i % 2 == 0)
            value = tex[uint2(i, 0)];
        output[i] = value;
    }
}

// Struct values also retain native phis, including the initial value and conditional update
// that really must survive to the next iteration.
// AGGREGATE: OpPhi %State
// AGGREGATE: OpPhi %State

struct State
{
    float4 value;
    float4 other;
};

RWStructuredBuffer<State> states;

[shader("compute")]
[numthreads(1, 1, 1)]
void aggregate(uint3 tid : SV_DispatchThreadID)
{
    State value = states[0];
    for (uint i = 0; i < tid.x; ++i)
    {
        if (i % 2 == 0)
            value = states[i];
        states[i + 100] = value;
    }
}
