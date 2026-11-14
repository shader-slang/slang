//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute

// `__floatAsInt` still folds a conversion of a literal to the floating-point type.

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // float(5) = 0x40A00000
    // CHECK: int(1084227584)
    outputBuffer[0] = __floatAsInt(float(5));

    // (float)1 = 0x3F800000
    // CHECK: int(1065353216)
    outputBuffer[1] = __floatAsInt((float)1);

    // float(2.0) = 0x40000000
    // CHECK: int(1073741824)
    outputBuffer[2] = __floatAsInt(float(2.0));

    // half(1) = 0x3C00
    // CHECK: 15360
    outputBuffer[3] = int(__floatAsInt(half(1)));
}
