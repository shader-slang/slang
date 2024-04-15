//DISABLED_TEST:SIMPLE(filecheck=CHECK1):-target spirv -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK2):-target spirv -entry main -stage compute -emit-spirv-directly

RWStructuredBuffer<int> buffer[];

[numthreads(8, 1, 1)]
void main(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    // CHECK1-DAG: OpDecorate %[[VAR1:[a-zA-Z0-9_]+]] NonUniform
    // CHECK1-DAG: OpDecorate %[[VAR2:[a-zA-Z0-9_]+]] NonUniform
    // CHECK1-DAG: OpDecorate %[[VAR3:[a-zA-Z0-9_]+]] NonUniform
    // CHECK1: %[[VAR1]] = OpCopyObject %{{.*}}
    // CHECK1: %[[VAR2]] = OpAccessChain %_ptr_Uniform_int %buffer_{{.*}} %[[VAR1]]
    // CHECK1: OpStore  %[[VAR2]] %{{.*}}
    // CHECK1: %[[VAR3]] = OpCopyObject %{{.*}}
    // CHECK1: %{{.*}} = OpBitcast %int %[[VAR3]]

    // CHECK2-DAG: OpDecorate %[[VAR1:[a-zA-Z0-9_]+]] NonUniform
    // CHECK2-DAG: OpDecorate %[[VAR2:[a-zA-Z0-9_]+]] NonUniform
    // CHECK2: %[[VAR1]] = OpAccessChain %_ptr_StorageBuffer_RWStructuredBuffer %buffer %{{.*}}
    // CHECK2: %{{.*}} = OpAccessChain %_ptr_StorageBuffer_int %[[VAR1]]
    // CHECK2: %[[VAR2]] = OpAccessChain %_ptr_StorageBuffer_RWStructuredBuffer %buffer %{{.*}}
    // CHECK2: %{{.*}} = OpAccessChain %_ptr_StorageBuffer_int %[[VAR2]]
    RWStructuredBuffer<int> buffer1 = buffer[NonUniformResourceIndex(NonUniformResourceIndex(NonUniformResourceIndex(dispatchThreadID.x)))];
    buffer1[0] = 1;

    RWStructuredBuffer<int> buffer2 = buffer[int(NonUniformResourceIndex(uint(float(dispatchThreadID.x))))];
    buffer2[0] = 1;
}