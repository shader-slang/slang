// A borrow parameter is read-only in the body, but reads still see the caller's argument,
// and a legacy (C-style) `in` parameter remains a mutable local copy.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

struct S { int a[4]; }

[noinline] int readBorrow(__constref S s) { return s.a[0]; }
[noinline] int bumpLegacyIn(in int x) { x = x + 10; return x; }

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    S s;
    s.a = { 1, 1, 1, 1 };
    outputBuffer[0] = readBorrow(s);
    outputBuffer[1] = s.a[0];
    int y = 5;
    outputBuffer[2] = bumpLegacyIn(y);
    outputBuffer[3] = y;
}
//CHECK: 1
//CHECK: 1
//CHECK: 15
//CHECK: 5
