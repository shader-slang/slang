//TEST:SIMPLE(filecheck=CHECK): -target spirv

// Regression test for gh-7262: [mutating] method on a struct containing Atomic<T>
// was generating an atomic operation on a Function-address-space local copy, producing
// invalid SPIR-V and crashing the GPU driver.
//
// The fix: structs containing Atomic fields are now non-copyable, so a [mutating]
// this-parameter is automatically promoted to [__ref] (true reference), bypassing
// the copy-in/copy-out that caused the bad atomic destination.

//CHECK: OpEntryPoint
//CHECK: OpAtomicFAddEXT

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
