// Quad-scoped wave operations: QuadReadAcrossX / Y / Diagonal and
// QuadReadLaneAt. These reduce across the fragment quad (2x2 group of
// fragments), distinct from full-wave reductions. The quad model is
// part of the execution model's wave/quad classification.

//TEST:SIMPLE(filecheck=CHECK_HLSL): -target hlsl -stage fragment -entry fragmentMain
//TEST:SIMPLE(filecheck=CHECK_GLSL): -target glsl -stage fragment -entry fragmentMain
//TEST:SIMPLE(filecheck=CHECK_SPIRV): -target spirv-asm -stage fragment -entry fragmentMain -emit-spirv-directly

[shader("fragment")]
float4 fragmentMain(float4 pos : SV_Position) : SV_Target
{
    float v = pos.x;
    float ax = QuadReadAcrossX(v);
    float ay = QuadReadAcrossY(v);
    float ad = QuadReadAcrossDiagonal(v);
    float at0 = QuadReadLaneAt(v, 0u);
    return float4(ax, ay, ad, at0);
}

// HLSL preserves the explicit intrinsics.
// CHECK_HLSL-DAG: QuadReadAcrossX
// CHECK_HLSL-DAG: QuadReadAcrossY
// CHECK_HLSL-DAG: QuadReadAcrossDiagonal
// CHECK_HLSL-DAG: QuadReadLaneAt

// GLSL emits subgroupQuad* variants from the quad subgroup extension.
// CHECK_GLSL-DAG: subgroupQuadSwapHorizontal
// CHECK_GLSL-DAG: subgroupQuadSwapVertical
// CHECK_GLSL-DAG: subgroupQuadSwapDiagonal
// CHECK_GLSL-DAG: subgroupQuadBroadcast

// SPIRV emits the OpGroupNonUniformQuad* opcodes.
// CHECK_SPIRV-DAG: OpGroupNonUniformQuadSwap
// CHECK_SPIRV-DAG: OpGroupNonUniformQuadBroadcast
