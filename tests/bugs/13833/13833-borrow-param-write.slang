// Regression test for #13833: a borrow parameter (`__constref`) is an immutable borrow of its
// argument, so the callee must not be able to write it. On targets that lower a borrow to a
// pointer (CPU, CUDA, SPIR-V, WGSL) a write used to reach the caller's storage.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

struct S { int a[4]; }

[noinline] int bump(__constref S s) { s.a[0] = 7; return s.a[0]; }
[noinline] int bumpInt(__constref int x) { x = 9; return x; }

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    S s;
    s.a = { 1, 1, 1, 1 };
    int x = 1;
    outputBuffer[0] = bump(s);
    outputBuffer[1] = s.a[0];
    outputBuffer[2] = bumpInt(x);
    outputBuffer[3] = x;
}
//CHECK: 7
//CHECK: 1
//CHECK: 9
//CHECK: 1
