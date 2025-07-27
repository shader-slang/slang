// Test that globallycoherent works on arrays of uavs.

//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry main -emit-spirv-directly
//TEST:SIMPLE(filecheck=VK_MEM_MODEL): -target spirv -stage compute -entry main -emit-spirv-directly -capability vk_mem_model

globallycoherent RWByteAddressBuffer buffer[];

RWStructuredBuffer<float> output;
[numthreads(4,1,1)]
void main(int tid : SV_DispatchThreadID)
{
    buffer[NonUniformResourceIndex(0)].InterlockedAdd(0, 1);
    AllMemoryBarrier();
    output[tid] = buffer[0].Load(0);
    // CHECK-DAG: OpDecorate %buffer Coherent
    // CHECK-DAG: OpDecorate %[[VAR1:[a-zA-Z0-9_]+]] NonUniform
    // CHECK: %[[VAR1]] = OpAccessChain %{{.*}} %buffer

    //VK_MEM_MODEL: OpLoad {{.*}} MakePointerVisible|NonPrivatePointer
}
