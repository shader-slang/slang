//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry computeMain -line-directive-mode none

// CHECK-NOT: unsigned
// CHECK: uint useUnsignedTypeNames_
// CHECK-SAME: (uint value_
// CHECK-NOT: unsigned
// CHECK: uint ShadowedUInt_acceptBuiltinUInt_
// CHECK-SAME: (uint value_
// CHECK-NOT: unsigned
// CHECK: void computeMain(
// CHECK-NOT: unsigned

uint acceptUInt(uint value)
{
    return value;
}

struct NotUInt
{};

NotUInt acceptUInt(int value)
{
    NotUInt result;
    return result;
}

static unsigned globalValue = 0;

namespace ShadowedUInt
{
struct uint
{};

unsigned acceptBuiltinUInt(unsigned value)
{
    return acceptUInt(value);
}
}

unsigned int useUnsignedTypeNames(unsigned value)
{
    unsigned int localValue = (unsigned int)value;
    unsigned int negativeShortCast = (unsigned)-1;
    unsigned int negativeLongCast = (unsigned int)-1;
    unsigned int positiveShortCast = (unsigned)+1;
    unsigned int positiveLongCast = (unsigned int)+1;
    vector<unsigned int, 4> vectorValue = localValue;
    vector<unsigned, 4> shortVectorValue = localValue;
    uint typeSize = sizeof(unsigned int);
    return acceptUInt(
        globalValue + vectorValue.x + shortVectorValue.x + typeSize + negativeShortCast +
        negativeLongCast + positiveShortCast + positiveLongCast);
}

RWStructuredBuffer<uint> output;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    output[dispatchThreadID.x] =
        useUnsignedTypeNames(dispatchThreadID.x) +
        ShadowedUInt::acceptBuiltinUInt(dispatchThreadID.x);
}
