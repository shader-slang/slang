// Regression test for GitHub issue #10505:
// GLSL emitter was missing the bitCast from UInt64/Int64 to Double,
// causing ICE E99999 when AnyValue unmarshalling generates
// bitCast(UInt64) -> Double.

//TEST:SIMPLE(filecheck=CHECK): -target glsl -stage compute -entry computeMain

interface IValue
{
    float asFloat();
}

struct DoubleImpl : IValue
{
    double val;
    float asFloat() { return float(val); }
}

struct FloatImpl : IValue
{
    float val;
    float asFloat() { return val; }
}

RWStructuredBuffer<float> outputBuffer;
StructuredBuffer<int> kindBuffer;

IValue makeValue(int kind)
{
    if (kind == 0) { DoubleImpl d; d.val = 3.25; return d; }
    FloatImpl f; f.val = 1.0; return f;
}

// CHECK: uint64BitsToDouble

[numthreads(1, 1, 1)]
void computeMain()
{
    IValue v = makeValue(kindBuffer[0]);
    outputBuffer[0] = v.asFloat();
}
