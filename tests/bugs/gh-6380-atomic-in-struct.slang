//DISABLE_TEST:SIMPLE(filecheck=CHECK): -target spirv
// Disable test until below error is resolved:
//error 41403: cannot perform atomic operation because destination is neither groupshared nor from a device buffer.
//    Atomic<int> test;
//                ^~~~

//CHECK: OpEntryPoint

struct Test {
    Atomic<int> test;
}

RWStructuredBuffer<Test> data;

[shader("compute")]
[numthreads(1,1,1)]
public void main() {
    Test t = Test(1);
    data[0].test.increment();
}