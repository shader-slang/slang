//TEST:SIMPLE(filecheck=CHECK): -target spirv

// Regression test for gh-7262: [mutating] method on a struct containing Atomic<T>
// was generating an atomic operation on a Function-address-space local copy, producing
// invalid SPIR-V and crashing the GPU driver.
//
// The fix: structs containing Atomic fields are now non-copyable, so a [mutating]
// this-parameter is automatically promoted to [__ref] (true reference), bypassing
// the copy-in/copy-out that caused the bad atomic destination.

//CHECK: OpEntryPoint
// Verify the atomic target is in StorageBuffer address space, not a Function-local copy.
// The capture ties the atomic's own pointer operand back to a StorageBuffer-class
// pointer type, so this fails if the atomic ever targets a Function-space local again
// (a plain "OpTypePointer StorageBuffer" CHECK-DAG would pass trivially, satisfied by
// the RWStructuredBuffer declaration itself, without checking the atomic's operand).
//CHECK-NOT: OpTypePointer Function %float
//CHECK-DAG: %[[SB_FLOAT_PTR:[a-zA-Z0-9_]+]] = OpTypePointer StorageBuffer %float
//CHECK: %[[ATOMIC_DEST:[a-zA-Z0-9_]+]] = OpAccessChain %[[SB_FLOAT_PTR]] {{.*}}
//CHECK: OpAtomicFAddEXT %float %[[ATOMIC_DEST]]

public struct TestAtomic {
    public Atomic<float> val;

    [mutating]
    public void add(float v) {
        this.val.add(v);
    }
}

RWStructuredBuffer<TestAtomic> test_atomic_buffer;

[shader("compute")]
[numthreads(64, 1, 1)]
void test_add(uint tid: SV_DispatchThreadID) {
    uint idx = tid / 2;
    test_atomic_buffer[idx].add(1.0f);
}
