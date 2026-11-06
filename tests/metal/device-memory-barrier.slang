// Device-scope memory barriers on Metal.
//
// `threadgroup_barrier` orders memory only among the threads of one threadgroup, so for a
// `metallib_3_2` (or newer) target `DeviceMemoryBarrier` and `AllMemoryBarrier` lower to a
// device-scope `atomic_thread_fence`, which needs MSL 3.2. The non-sync forms are then not an
// execution barrier, and the `WithGroupSync` forms keep the threadgroup barrier after the fence.
// Older targets, and a target without a metallib version, keep `threadgroup_barrier`.

//TEST:SIMPLE(filecheck=MTL32): -target metal -stage compute -entry computeMain -capability metallib_3_2
//TEST:SIMPLE(filecheck=MTL32): -target metal -stage compute -entry computeMain -capability metallib_4_0
//TEST:SIMPLE(filecheck=MTLOLD): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=MTLOLD): -target metal -stage compute -entry computeMain -capability metallib_3_1
//TEST:SIMPLE(filecheck=LIB): -target metallib -stage compute -entry computeMain
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-mtl -compute -shaderobj -output-using-type

// MTL32-NOT: threadgroup_barrier
// DeviceMemoryBarrier
// MTL32: atomic_thread_fence(mem_flags::mem_device | mem_flags::mem_texture, memory_order_seq_cst, thread_scope_device)
// MTL32-NOT: threadgroup_barrier
// AllMemoryBarrier
// MTL32: atomic_thread_fence(mem_flags::mem_device | mem_flags::mem_threadgroup | mem_flags::mem_texture, memory_order_seq_cst, thread_scope_device)
// MTL32-NOT: threadgroup_barrier
// DeviceMemoryBarrierWithGroupSync
// MTL32: atomic_thread_fence(mem_flags::mem_device | mem_flags::mem_texture, memory_order_seq_cst, thread_scope_device)
// MTL32-SAME: threadgroup_barrier(
// AllMemoryBarrierWithGroupSync
// MTL32: atomic_thread_fence(mem_flags::mem_device | mem_flags::mem_threadgroup | mem_flags::mem_texture, memory_order_seq_cst, thread_scope_device)
// MTL32-SAME: threadgroup_barrier(

// MTLOLD-NOT: atomic_thread_fence
// MTLOLD-COUNT-4: threadgroup_barrier(mem_flags::mem_device | mem_flags::mem_threadgroup | mem_flags::mem_texture | mem_flags::mem_threadgroup_imageblock)
// MTLOLD-NOT: atomic_thread_fence

// LIB: computeMain

//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4):out,name=nodes
globallycoherent RWStructuredBuffer<uint> nodes;
//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4):name=visits
RWStructuredBuffer<uint> visits;

[numthreads(4, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    uint i = tid.x;
    nodes[i] = i + 1;
    DeviceMemoryBarrier();
    uint prev;
    InterlockedAdd(visits[i / 2], 1, prev);
    AllMemoryBarrier();
    nodes[i] += 10;
    DeviceMemoryBarrierWithGroupSync();
    nodes[i] += 100;
    AllMemoryBarrierWithGroupSync();
    if (i == 0)
        nodes[0] = nodes[0] + nodes[1] + nodes[2] + nodes[3];
}

// (1 + 110) + (2 + 110) + (3 + 110) + (4 + 110) = 450
// BUF: 450
// BUF-NEXT: 112
// BUF-NEXT: 113
// BUF-NEXT: 114
