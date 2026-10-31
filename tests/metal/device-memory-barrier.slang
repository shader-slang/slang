// Device-scope memory barriers on Metal.
//
// `threadgroup_barrier` orders memory only among the threads of one threadgroup, so
// `DeviceMemoryBarrier` and `AllMemoryBarrier` lower to a device-scope `atomic_thread_fence`
// (MSL 3.2). The non-sync forms are not an execution barrier, and the `WithGroupSync` forms keep
// the threadgroup barrier after the fence. A target capped below `metallib_3_2` is diagnosed
// like any other capability upgrade.

//TEST:SIMPLE(filecheck=MTL): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=MTL): -target metal -stage compute -entry computeMain -capability metallib_3_2
//TEST:SIMPLE(filecheck=LIB): -target metallib -stage compute -entry computeMain
//DIAGNOSTIC_TEST:SIMPLE(filecheck=WARN): -target metal -stage compute -entry computeMain -capability metallib_3_1 -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(filecheck=ERROR): -target metal -stage compute -entry computeMain -capability metallib_3_1 -no-codegen -restrictive-capability-check
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-mtl -compute -shaderobj -output-using-type

// MTL-NOT: threadgroup_barrier
// DeviceMemoryBarrier
// MTL: atomic_thread_fence(mem_flags::mem_device | mem_flags::mem_texture, memory_order_seq_cst, thread_scope_device)
// MTL-NOT: threadgroup_barrier
// AllMemoryBarrier
// MTL: atomic_thread_fence(mem_flags::mem_device | mem_flags::mem_threadgroup | mem_flags::mem_texture, memory_order_seq_cst, thread_scope_device)
// MTL-NOT: threadgroup_barrier
// DeviceMemoryBarrierWithGroupSync
// MTL: atomic_thread_fence(mem_flags::mem_device | mem_flags::mem_texture, memory_order_seq_cst, thread_scope_device)
// MTL-SAME: threadgroup_barrier(
// AllMemoryBarrierWithGroupSync
// MTL: atomic_thread_fence(mem_flags::mem_device | mem_flags::mem_threadgroup | mem_flags::mem_texture, memory_order_seq_cst, thread_scope_device)
// MTL-SAME: threadgroup_barrier(

// LIB: computeMain

// WARN: warning[E41012]
// WARN: capabilities: 'metallib_3_2'
// ERROR: error[E41013]
// ERROR: capabilities are: 'metallib_3_2'

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
