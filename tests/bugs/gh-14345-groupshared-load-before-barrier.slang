// Regression test for #14345: a load from groupshared memory that the source places before a
// barrier must be emitted before that barrier. The emitter moves a load to its use site when no
// instruction in between can change the loaded memory. A call with side effects was only assumed
// to change globals, so a load through a pointer (the kernel context on Metal, or a pointer
// parameter) was moved past the barrier call.

//TEST:SIMPLE(filecheck=METAL): -target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry main -stage compute

groupshared float sh[64];

// On Metal, `sh` is reached through a pointer in the kernel context.
// METAL-LABEL: float readSharedThenSync_0(
// METAL: float [[T:[A-Za-z_0-9]+]] = {{.*}}sh_0{{.*}}[int(0)];
// METAL-NEXT: threadgroup_barrier(mem_flags::mem_threadgroup);
// METAL-NEXT: return [[T]];
float readSharedThenSync()
{
    float total = sh[0];
    GroupMemoryBarrierWithGroupSync(); // `sh` may be written again after this barrier
    return total;
}

// A pointer parameter on every target that supports it.
// METAL-LABEL: float readPtrThenSync_0(
// METAL: float [[P:[A-Za-z_0-9]+]] = *p_0;
// METAL-NEXT: threadgroup_barrier(mem_flags::mem_threadgroup);
// METAL-NEXT: return [[P]];
// CUDA-LABEL: float readPtrThenSync_0(
// CUDA: float [[P:[A-Za-z_0-9]+]] = *p_0;
// CUDA-NEXT: __syncthreads();
// CUDA-NEXT: return [[P]];
float readPtrThenSync(Ptr<float, Access::ReadWrite, AddressSpace::GroupShared> p)
{
    float t = *p;
    GroupMemoryBarrierWithGroupSync();
    return t;
}

// Redundant store removal must keep the first store: other threads read `*p` after the
// barrier, so the barrier call has to count as an access to `*p`.
// METAL-LABEL: void writeTwiceAcrossSync_0(
// METAL-SAME: * [[Q:p_[0-9]+]])
// METAL: *[[Q]] = 1.0f;
// METAL-NEXT: threadgroup_barrier(mem_flags::mem_threadgroup);
// METAL-NEXT: *[[Q]] = 2.0f;
// CUDA-LABEL: void writeTwiceAcrossSync_0(
// CUDA-SAME: * [[Q:p_[0-9]+]])
// CUDA: *[[Q]] = 1.0f;
// CUDA-NEXT: __syncthreads();
// CUDA-NEXT: *[[Q]] = 2.0f;
void writeTwiceAcrossSync(Ptr<float, Access::ReadWrite, AddressSpace::GroupShared> p)
{
    *p = 1.0f;
    GroupMemoryBarrierWithGroupSync(); // other threads read `*p` here
    *p = 2.0f;
}

RWStructuredBuffer<float> io;

[shader("compute")]
[numthreads(64, 1, 1)]
void main(uint3 gtid: SV_GroupThreadID)
{
    sh[gtid.x] = io[gtid.x];
    GroupMemoryBarrierWithGroupSync();
    float a = readSharedThenSync();
    sh[gtid.x] = a + 1.0f;
    GroupMemoryBarrierWithGroupSync();
    float b = readPtrThenSync(__getAddress(sh[0]));
    sh[gtid.x] = b + 2.0f;
    GroupMemoryBarrierWithGroupSync();
    writeTwiceAcrossSync(__getAddress(sh[1]));
    b += sh[1];
    io[gtid.x] = a + b;
}
