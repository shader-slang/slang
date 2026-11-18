//TEST:COMPARE_COMPUTE(targets=all, filecheck-buffer=CHK):

//CHK:1

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint r1 = countbits(0b1);
    uint2 r2 = countbits(uint2(0b0, 0b1));
    uint3 r3 = countbits(uint3(0b0, 0b1, 0b11));
    uint4 r4 = countbits(uint4(0b0, 0b1, 0b11, 0b111));

    outputBuffer[0] = true
        && (r1 == 1)
        && (r2.x == 0 && r2.y == 1)
        && (r3.x == 0 && r3.y == 1 && r3.z == 2)
        && (r4.x == 0 && r4.y == 1 && r4.z == 2 && r4.w == 3)
        ;
}
