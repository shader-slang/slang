//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Regression for shader-slang/slang#11844: empty/partial initialization of a
// struct that mixes bitfields with a trailing normal field must zero-initialize
// the hidden bitfield backing storage, instead of leaving it uninitialized.

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

struct Test
{
    uint a : 10;
    uint b : 22;
    uint c;
};

[numthreads(1, 1, 1)]
void computeMain()
{
    // Empty initializer: every field, including the bitfield backing word, is zero.
    Test t0 = {};
    // CHECK: 0
    // CHECK-NEXT: 0
    // CHECK-NEXT: 0
    outputBuffer[0] = t0.a;
    outputBuffer[1] = t0.b;
    outputBuffer[2] = t0.c;

    // Partial initializer: only the normal field 'c' is supplied; the bitfields
    // must still read as zero because the backing word is zero-initialized.
    Test t1 = {7};
    // CHECK-NEXT: 0
    // CHECK-NEXT: 0
    // CHECK-NEXT: 7
    outputBuffer[3] = t1.a;
    outputBuffer[4] = t1.b;
    outputBuffer[5] = t1.c;
}
