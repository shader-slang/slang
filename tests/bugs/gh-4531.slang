//TEST:COMPARE_COMPUTE(targets=vk+dx12+metal, filecheck-buffer=CHECK):

// CHECK:      1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint tid : SV_GroupIndex)
{
    vector<bool,4> k = true;
    outputBuffer[tid] = all(k);
}
