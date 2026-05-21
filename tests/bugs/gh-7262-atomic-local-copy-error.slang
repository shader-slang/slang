//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

// Regression guard: creating a local variable of a struct that contains Atomic<T>
// is now an error, because Atomic<T> is [__NonCopyableType] and therefore cannot
// be placed in Function address space.
//
// This was previously silently accepted (before gh-7262) and produced invalid SPIR-V
// (atomic operation on a Function-address-space variable).

struct Test {
    Atomic<int> test;
//CHECK:        ^^^^ invalid atomic destination
//CHECK:        ^^^^ cannot perform atomic operation because destination is neither groupshared nor from a device buffer.
}

RWStructuredBuffer<Test> data;

[shader("compute")]
[numthreads(1, 1, 1)]
public void main() {
    Test t = Test(1);
    data[0].test.increment();
}
