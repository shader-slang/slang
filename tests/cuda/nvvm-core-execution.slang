// Exercises canonical CUDA execution state, shared storage, aggregate addressing, and a group
// barrier through one raw direct-NVVM kernel.

//TEST:SIMPLE(filecheck=CHECK): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_7_0

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
// CHECK: .shared
// CHECK: mov.u32 {{.*}}%tid.x
// CHECK: bar.sync
// CHECK: st.global.u32
