//TEST:SIMPLE(filecheck=CHECK):-target spirv -emit-spirv-directly -entry computeMain -stage compute -capability cooperative_vector -O0

// Check that cooperative vector StructuredBuffer overloads preserve distinct
// std140/std430 ArrayStride decorations for runtime arrays and pointers, and
// that the loaded value actually flows into the matching store (not swapped)
// even though `coopVecLoad`/`.store` are not inlined into `computeMain`.

// CHECK-DAG: OpDecorate %[[ARRAY_140:[A-Za-z0-9_]+]] ArrayStride 16
// CHECK-DAG: OpDecorate %[[ARRAY_430:[A-Za-z0-9_]+]] ArrayStride 4
// CHECK-DAG: OpDecorate %[[PTR_ARRAY_140:[A-Za-z0-9_]+]] ArrayStride 16
// CHECK-DAG: OpDecorate %[[PTR_ARRAY_430:[A-Za-z0-9_]+]] ArrayStride 4
// CHECK-DAG: %[[COOP_VEC:[A-Za-z0-9_]+]] = OpTypeVectorIdEXT %float %int_4
// CHECK-DAG: %[[ARRAY_140]] = OpTypeRuntimeArray %float
// CHECK-DAG: %[[ARRAY_430]] = OpTypeRuntimeArray %float
// CHECK-DAG: %[[PTR_ARRAY_140]] = OpTypePointer StorageBuffer %[[ARRAY_140]]
// CHECK-DAG: %[[PTR_ARRAY_430]] = OpTypePointer StorageBuffer %[[ARRAY_430]]

// `computeMain` is defined (and thus appears in the disassembly) before the
// load/store helper functions it calls, so this value-flow check -- which
// re-binds the load->store value at the call sites, since the load/store
// opcodes themselves live in those separately-compiled helpers -- has to
// come before the checks that look inside those helper bodies below.
// CHECK: %[[VAL_140:[A-Za-z0-9_]+]] = OpFunctionCall %[[COOP_VEC]] %CoopVec_load %int_0
// CHECK: %[[VAL_430:[A-Za-z0-9_]+]] = OpFunctionCall %[[COOP_VEC]] %CoopVec_load_0 %int_0
// CHECK: OpFunctionCall %void %CoopVec_store %[[VAL_140]] %int_0
// CHECK: OpFunctionCall %void %CoopVec_store_0 %[[VAL_430]] %int_0

// CHECK: %[[INPUT_140:[A-Za-z0-9_]+]] = OpAccessChain %[[PTR_ARRAY_140]] %input140 %int_0
// CHECK: OpCooperativeVectorLoadNV %[[COOP_VEC]] %[[INPUT_140]] {{.*}} None
// CHECK: %[[INPUT_430:[A-Za-z0-9_]+]] = OpAccessChain %[[PTR_ARRAY_430]] %input430 %int_0
// CHECK: OpCooperativeVectorLoadNV %[[COOP_VEC]] %[[INPUT_430]] {{.*}} None
// CHECK: %[[OUTPUT_140:[A-Za-z0-9_]+]] = OpAccessChain %[[PTR_ARRAY_140]] %output140 %int_0
// CHECK: OpCooperativeVectorStoreNV %[[OUTPUT_140]] {{.*}} None
// CHECK: %[[OUTPUT_430:[A-Za-z0-9_]+]] = OpAccessChain %[[PTR_ARRAY_430]] %output430 %int_0
// CHECK: OpCooperativeVectorStoreNV %[[OUTPUT_430]] {{.*}} None


StructuredBuffer<float, Std140DataLayout> input140;
RWStructuredBuffer<float, Std140DataLayout> output140;

StructuredBuffer<float, Std430DataLayout> input430;
RWStructuredBuffer<float, Std430DataLayout> output430;

[numthreads(1, 1, 1)]
void computeMain()
{
    let a = coopVecLoad<4, float>(input140, 0);
    let b = coopVecLoad<4, float>(input430, 0);

    a.store(output140, 0);
    b.store(output430, 0);
}
