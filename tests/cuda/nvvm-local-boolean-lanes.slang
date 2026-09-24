//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

//TEST_INPUT:ubuffer(data=[0 1 2 3], stride=4):name=inputBuffer
StructuredBuffer<uint> inputBuffer;
//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint index = inputBuffer[tid.x];
    bool2 pair = bool2(true, false);
    bool3 triple = bool3(false, true, false);
    bool4 quad = bool4(true, false, true, false);
    pair[index % 2] = !pair[index % 2];
    triple[index % 3] = !triple[index % 3];
    quad[index] = !quad[index];
    // Repeated dynamic access observes the updated lane while whole-vector reads retain neighbors.
    uint result = uint(pair[0]) | (uint(pair[1]) << 1);
    result |= (uint(triple[0]) | (uint(triple[1]) << 1) | (uint(triple[2]) << 2)) << 2;
    result |= (uint(quad[0]) | (uint(quad[1]) << 1) | (uint(quad[2]) << 2) | (uint(quad[3]) << 3)) << 5;
    result |= uint(quad[index]) << 9;
    // Every lane is initialized once in a runtime-dependent order before the value is observed.
    bool4 initialized;
    for (uint step = 0; step < 4; ++step)
    {
        uint lane = (index + step) % 4;
        initialized[lane] = (lane % 2) == 0;
    }
    uint initializedMask = uint(initialized[0]) | (uint(initialized[1]) << 1);
    initializedMask |= (uint(initialized[2]) << 2) | (uint(initialized[3]) << 3);
    outputBuffer[tid.x] = result | (initializedMask << 10);
}
// CHECK: 5260
// CHECK-NEXT: 5859
// CHECK-NEXT: 5176
// CHECK-NEXT: 6063
