//TEST:SIMPLE(filecheck=CHECK): -target spirv

// Verify that Atomic<T> inside a struct accessed via RWStructuredBuffer compiles correctly.
// The earlier failure was error 41403 (atomic on local variable); fixed by making
// structs containing Atomic fields non-copyable so [mutating] is promoted to [__ref].

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
