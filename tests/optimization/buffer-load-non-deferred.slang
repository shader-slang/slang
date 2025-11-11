//TEST:SIMPLE(filecheck=SPV13): -target spirv -profile spirv_1_3
//TEST:SIMPLE(filecheck=SPV): -target spirv

// Check that we are able to use OpCopyLogical to simplify loading a physical type into a local variable.

struct Bottom
{
    float bigArray[1024];
    float bottomGetValue(int index) { return bigArray[index]; }
}

struct Middle
{
    Bottom bottom;
    float middleGetValue(int index) { return bottom.bottomGetValue(index); }
}

uniform Middle* cb;

RWStructuredBuffer<float> outputBuffer;

// SPV: OpEntryPoint
// SPV13: OpEntryPoint
[shader("compute")]
[numthreads(1, 1, 1)]
void compute_main(uint3 tid: SV_DispatchThreadID)
{
    Middle v = *cb;
    // SPV: OpCopyLogical
    // SPV13-NOT: OpCopyLogical
    outputBuffer[0] = v.middleGetValue(0);
    cb.bottom.bigArray[0] = 42.0;
    outputBuffer[1] = v.middleGetValue(0);

}
