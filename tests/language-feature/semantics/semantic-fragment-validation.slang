//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage fragment -entry fragmentMain

// Test that fragment-only semantics are validated correctly

struct FragmentInput
{
    float4 position : SV_Position;
    bool isFrontFace : SV_IsFrontFace;  // Valid in fragment stage
    uint sampleIndex : SV_SampleIndex;  // Valid in fragment stage
};

struct FragmentOutput
{
    float4 color : SV_Target0;
    float depth : SV_Depth;  // Valid in fragment stage
};

[shader("fragment")]
FragmentOutput fragmentMain(FragmentInput input)
{
    FragmentOutput output;
    output.color = float4(1, 0, 0, 1);
    output.depth = input.position.z;
    return output;
}

// CHECK: fragmentMain

