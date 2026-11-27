// existential-short-circuit.slang

// Test that a method call on an existential in an operand that is skipped (the right side of
// `&&`/`||`, or an arm of `?:`) does not evaluate its receiver.

//TEST:COMPARE_COMPUTE(targets=all, filecheck-buffer=CHECK):-output-using-type

interface IPred
{
    bool test();
    bool testN<let N : int>();
}

struct Pred : IPred
{
    int v;
    bool test() { return v != 0; }
    bool testN<let N : int>() { return v == N; }
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

// Create a receiver and count how often the one for `slot` is evaluated.
IPred make(int slot)
{
    outputBuffer[slot] += 1;
    return Pred(1);
}

// Receivers 0-5 are in skipped operands and stay 0, receivers 6-11 are in evaluated operands and
// are evaluated once, and slot 12 is 1 when every result is correct.
// CHECK: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1

[numthreads(1, 1, 1)]
void computeMain(int3 tid : SV_DispatchThreadID)
{
    bool f = tid.x != 0;
    bool t = tid.x == 0;

    // Skipped operands.
    bool r0 = f && make(0).test();
    bool r1 = f && make(1).testN<1>();
    bool r2 = t || make(2).test();
    bool r3 = t || make(3).testN<1>();

    // Evaluated operands.
    bool r4 = t && make(8).test();
    bool r5 = t && make(9).testN<1>();
    bool r6 = f || make(10).test();
    bool r7 = f || make(11).testN<1>();

    // One arm of each `?:` is skipped and the other is evaluated.
    bool r8 = t ? make(6).test() : make(4).test();
    bool r9 = f ? make(5).testN<1>() : make(7).testN<1>();

    outputBuffer[12] = (!r0 && !r1 && r2 && r3 && r4 && r5 && r6 && r7 && r8 && r9) ? 1 : 0;
}
