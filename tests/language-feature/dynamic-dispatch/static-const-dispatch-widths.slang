// Static const dynamic dispatch for non-default integer widths.
//
// The main static-const-dispatch test covers int and bool across broad targets.
// This focused CPU test covers narrower, wider, and unsigned value-set entries
// without requiring every backend in the broad test to support 64-bit integers.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=IR):-target spirv-asm -stage compute -entry computeMain -dump-ir-after specializeModule

interface IWideLabels
{
    static const int8_t smallSigned;
    static const int64_t wideSigned;
    static const uint64_t wideUnsigned;
}

struct WideA : IWideLabels
{
    static const int8_t smallSigned = int8_t(-7);
    static const int64_t wideSigned = -42;
    static const uint64_t wideUnsigned = (uint64_t(5) << 32) | uint64_t(99);
}

struct WideB : IWideLabels
{
    static const int8_t smallSigned = int8_t(12);
    static const int64_t wideSigned = (int64_t(3) << 32) | int64_t(77);
    static const uint64_t wideUnsigned = (uint64_t(9) << 32) | uint64_t(11);
}

int getSmallSigned(IWideLabels op)
{
    return int(op.smallSigned);
}

int getWideSignedLow(IWideLabels op)
{
    return int(op.wideSigned & int64_t(0xFFFFFFFF));
}

int getWideSignedHigh(IWideLabels op)
{
    return int(op.wideSigned >> 32);
}

int getWideUnsignedLow(IWideLabels op)
{
    return int(op.wideUnsigned & uint64_t(0xFFFFFFFF));
}

int getWideUnsignedHigh(IWideLabels op)
{
    return int(op.wideUnsigned >> 32);
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    IWideLabels a;
    if (outputBuffer[0] == 0)
        a = WideA();
    else
        a = WideB();

    IWideLabels b;
    if (outputBuffer[1] == 999)
        b = WideA();
    else
        b = WideB();

    outputBuffer[0] = getSmallSigned(a);    // CHECK: -7
    outputBuffer[1] = getWideSignedLow(a);  // CHECK: -42
    outputBuffer[2] = getWideSignedHigh(a); // CHECK: -1
    outputBuffer[3] = getWideUnsignedLow(a);  // CHECK: 99
    outputBuffer[4] = getWideUnsignedHigh(a); // CHECK: 5

    outputBuffer[5] = getSmallSigned(b);    // CHECK: 12
    outputBuffer[6] = getWideSignedLow(b);  // CHECK: 77
    outputBuffer[7] = getWideSignedHigh(b); // CHECK: 3
    outputBuffer[8] = getWideUnsignedLow(b);  // CHECK: 11
    outputBuffer[9] = getWideUnsignedHigh(b); // CHECK: 9
}

// IR: ### AFTER specializeModule:
// IR-DAG: GetTagForMappedValueSet({{.*}}, %smallSigned)
// IR-DAG: GetTagForMappedValueSet({{.*}}, %wideSigned)
// IR-DAG: GetTagForMappedValueSet({{.*}}, %wideUnsigned)
// IR-NOT: lookupWitness({{.*}}, %smallSigned)
// IR-NOT: lookupWitness({{.*}}, %wideSigned)
// IR-NOT: lookupWitness({{.*}}, %wideUnsigned)
