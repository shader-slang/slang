// Exercises canonical CUDA execution state, shared storage, aggregate addressing, and a group
// barrier through one raw direct-NVVM kernel.

//TEST:SIMPLE(filecheck=CHECK): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_7_0 -o -

groupshared int sharedValues[64];

[CUDAKernel]
void computeMain(uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination)
{
    int index = int(cudaThreadIdx().x);
    sharedValues[index] = index * 3 + 1;
    GroupMemoryBarrierWithGroupSync();
    destination[index] = sharedValues[63 - index];
}

// CHECK: .visible .entry computeMain
// CHECK-DAG: .shared
// CHECK-DAG: mov.u32 {{.*}}%tid.x
// CHECK-DAG: bar.sync
// CHECK-DAG: st.global.u32
