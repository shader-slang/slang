// Fragment shader system value coverage: SV_Position, SV_IsFrontFace,
// SV_SampleIndex, SV_PrimitiveID, SV_Coverage. Each carries
// stage-specific lowering rules that distinguish fragment from other
// stages.

//TEST:SIMPLE(filecheck=CHECK_HLSL): -target hlsl -stage fragment -entry fragmentMain
//TEST:SIMPLE(filecheck=CHECK_GLSL): -target glsl -stage fragment -entry fragmentMain
//TEST:SIMPLE(filecheck=CHECK_SPIRV): -target spirv-asm -stage fragment -entry fragmentMain -emit-spirv-directly

struct FragInput
{
    float4 position : SV_Position;
    bool isFrontFace : SV_IsFrontFace;
    uint sampleIndex : SV_SampleIndex;
    uint primId : SV_PrimitiveID;
};

[shader("fragment")]
float4 fragmentMain(FragInput input, out uint coverage : SV_Coverage) : SV_Target
{
    coverage = input.isFrontFace ? 0xFFu : 0u;
    return float4(input.position.x, float(input.sampleIndex), float(input.primId), 1.0);
}

// HLSL: each semantic surfaces literally.
// CHECK_HLSL-DAG: SV_Position
// CHECK_HLSL-DAG: SV_IsFrontFace
// CHECK_HLSL-DAG: SV_SampleIndex
// CHECK_HLSL-DAG: SV_PrimitiveID
// CHECK_HLSL-DAG: SV_Coverage

// GLSL: each lowers to a specific built-in.
// CHECK_GLSL-DAG: gl_FragCoord
// CHECK_GLSL-DAG: gl_FrontFacing
// CHECK_GLSL-DAG: gl_SampleID
// CHECK_GLSL-DAG: gl_PrimitiveID
// CHECK_GLSL-DAG: gl_SampleMask

// SPIRV: each lowers to a specific BuiltIn decoration.
// CHECK_SPIRV-DAG: BuiltIn FragCoord
// CHECK_SPIRV-DAG: BuiltIn FrontFacing
// CHECK_SPIRV-DAG: BuiltIn SampleId
// CHECK_SPIRV-DAG: BuiltIn PrimitiveId
// CHECK_SPIRV-DAG: BuiltIn SampleMask
