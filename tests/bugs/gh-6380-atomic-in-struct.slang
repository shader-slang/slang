//TEST:SIMPLE(filecheck=CHECK): -target spirv

// Verify that Atomic<T> inside a struct accessed via RWStructuredBuffer compiles correctly.
// The earlier failure was error 41403 (atomic on local variable); fixed by promoting
// [mutating] this-parameters on types containing Atomic<T> fields to [__ref] semantics.

//CHECK: OpEntryPoint
// Verify the atomic target is in StorageBuffer address space, not a Function-local copy.
// The capture ties the atomic's own pointer operand back to a StorageBuffer-class
// pointer type, so this fails if the atomic ever targets a Function-space local again
// (a plain "OpTypePointer StorageBuffer" CHECK-DAG would pass trivially, satisfied by
// the RWStructuredBuffer declaration itself, without checking the atomic's operand).
//CHECK-NOT: OpTypePointer Function %int
//CHECK-DAG: %[[SB_INT_PTR:[a-zA-Z0-9_]+]] = OpTypePointer StorageBuffer %int
//CHECK: %[[ATOMIC_DEST:[a-zA-Z0-9_]+]] = OpAccessChain %[[SB_INT_PTR]] {{.*}}
//CHECK: OpAtomicIIncrement %int %[[ATOMIC_DEST]]

struct Test {
    Atomic<int> test;
}

RWStructuredBuffer<Test> data;

[shader("compute")]
[numthreads(1,1,1)]
public void main() {
    data[0].test.increment();
}
