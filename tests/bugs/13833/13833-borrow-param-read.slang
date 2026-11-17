// A read through a borrow parameter yields the argument's value, and the callee cannot
// disturb the caller's storage: `readBorrow` returns 1 and `s.a[0]` stays 1 afterwards. These
// values pin the observable contract; they cannot by themselves distinguish a borrow from a
// copy of the argument. A legacy (C-style) `in` parameter remains a mutable local copy:
// `bumpLegacyIn` returns 15 while the caller's `y` stays 5.

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
