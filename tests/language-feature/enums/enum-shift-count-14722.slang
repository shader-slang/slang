// Regression test for shader-slang/slang#14722: an enum-typed expression used as
// the shift count of `<<`/`>>` coerces to the enum's tag type through the
// concrete per-type shift overloads, the same argument-coercion path that
// already lets `+` and `|` take an enum operand. Before the fix this failed
// with `no overload for '<<' applicable to arguments of type (int, Options)`.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

enum Options
{
    Shift = 6,
    Mask = 1 << Options.Shift,
}

static const int kMask = 1 << Options.Shift;

uint64_t shiftLeft(uint64_t value, Options count)
{
    return value << count;
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK: 64
    outputBuffer[0] = Options.Mask;
    // CHECK: 64
    outputBuffer[1] = kMask;
    // CHECK: 64
    outputBuffer[2] = 1 << Options.Shift;
    // CHECK: 16
    outputBuffer[3] = 1024 >> Options.Shift;
    // CHECK: 64
    outputBuffer[4] = int(1u << Options.Shift);
    // CHECK: 256
    outputBuffer[5] = int(shiftLeft(4, Options.Shift));
    // CHECK: 32
    outputBuffer[6] = __shl(1, Options.Shift) >> 1;
    // CHECK: 16
    outputBuffer[7] = int((int4(1, 1, 1, 1) << Options.Shift).w >> 2);
}
