//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

// Positive/negative infinity, NaN and signed zero are supplied as runtime bit patterns.
//TEST_INPUT:ubuffer(data=[0x7f800000 0xff800000 0x7fc00000 0x80000000 0x3f800000 0xbf800000], stride=4):name=inputBuffer
StructuredBuffer<uint> inputBuffer;
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(6, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint index = tid.x;
    float3 values = float3(asfloat(inputBuffer[index]), asfloat(inputBuffer[(index + 1) % 6]), 1.0);
    bool3 nanLanes = isnan(values);
    bool3 infLanes = isinf(values);
    uint result = uint(nanLanes[0]) | (uint(nanLanes[1]) << 1) | (uint(nanLanes[2]) << 2);
    result |= (uint(infLanes[0]) | (uint(infLanes[1]) << 1) | (uint(infLanes[2]) << 2)) << 3;
    result |= uint(any(nanLanes) || any(infLanes)) << 6;
    outputBuffer[index] = result;
}
// CHECK: 88
// CHECK-NEXT: 74
// CHECK-NEXT: 65
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 80
