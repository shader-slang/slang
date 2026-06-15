//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage compute -emit-spirv-directly

// Test that NonUniform propagates through the image-atomic path on a
// MULTISAMPLED texture array, where the emitted OpImageTexelPointer carries a
// real (dynamic) sample operand rather than the default sample 0. This exercises
// the float pass's handling of the sample operand of kIROp_ImageTexelPointer,
// which the non-multisampled image-atomic tests (sample = 0) do not.

// CHECK-DAG: OpCapability ShaderNonUniform
// CHECK-DAG: OpCapability StorageImageMultisample
// CHECK-DAG: OpCapability StorageImageArrayNonUniformIndexing

// CHECK-DAG: %[[AC:[A-Za-z0-9_]+]] = OpAccessChain %{{[A-Za-z0-9_]+}} %images %[[IDX:[A-Za-z0-9_]+]]
// CHECK-DAG: OpDecorate %[[IDX]] NonUniform
// CHECK-DAG: OpDecorate %[[AC]] NonUniform
// The texel pointer has three operands (image, coord, sample); the sample
// operand is the dynamic index 's', not the constant 0.
// CHECK-DAG: %[[PTR:[A-Za-z0-9_]+]] = OpImageTexelPointer %{{[A-Za-z0-9_]+}} %[[AC]] %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}}
// CHECK-DAG: OpDecorate %[[PTR]] NonUniform

RWTexture2DMSArray<uint> images[8];

cbuffer Params { uint idx; uint s; };

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    uint i = idx + tid.x;
    InterlockedAdd(images[NonUniformResourceIndex(i)][uint3(0, 0, 0), s], 1);
}
