//TEST:SIMPLE(filecheck=CHECK): -target spirv

// Verify that Atomic<T> inside a struct accessed via RWStructuredBuffer compiles correctly.
// The earlier failure was error 41403 (atomic on local variable); fixed by promoting
// [mutating] this-parameters on types containing Atomic<T> fields to [__ref] semantics.

//CHECK: OpEntryPoint

struct Test {
    Atomic<int> test;
}

RWStructuredBuffer<Test> data;

[shader("compute")]
[numthreads(1,1,1)]
public void main() {
    data[0].test.increment();
}
