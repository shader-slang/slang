//TEST:COMPARE_COMPUTE(targets=cuda):
//TEST:COMPARE_COMPUTE(targets=cpu):-compile-arg -O3
//TEST:COMPARE_COMPUTE(targets=all):
//TEST:COMPARE_COMPUTE(targets=dx12):
//TEST:COMPARE_COMPUTE(targets=vk):


//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

void writeArray(inout float3 a[4])
{
    a[0] = float3(1, 1, 1);
    a[1] = float3(1, 1, 1);
    a[2] = float3(1, 1, 1);
    a[3] = float3(1, 1, 1);
}

[numthreads(4, 1, 1)]
void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
{
    float3 b[4];
    writeArray(b);
	outputBuffer[dispatchThreadID.x] = int(b[0].x);
}
