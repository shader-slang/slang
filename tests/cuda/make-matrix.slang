//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -compute 
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute 
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute -capability cuda_sm_7_0 -Xslang -emit-cuda-via-nvvm
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute -capability cuda_sm_7_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint4x3> outputBuffer : register(u0);

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint idx = dispatchThreadID.x + 1;
    uint4x3 mat1 = uint4x3(idx, idx, idx, idx, idx, idx, idx, idx, idx, idx, idx, idx);
    outputBuffer[0] = mat1;
    // CHECK: 1
}
