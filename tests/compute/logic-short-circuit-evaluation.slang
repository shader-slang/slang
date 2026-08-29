//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHK):-dx12 -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHK):-vk -compute  -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHK):-mtl -compute  -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHK):-cuda -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHK):-cuda -compute -shaderobj -capability cuda_sm_7_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHK):-cpu -compute -compile-arg -O3 -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHK):-slang -compute -shaderobj
//TEST:SIMPLE(filecheck=NVVM): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_7_0 -O3 -o -

//NVVM: setp.gt.s32
//NVVM: st.global.u32
//NVVM: setp.lt.s32
//NVVM: st.global.u32

// Test doing vector comparisons 

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

bool assignFunc(int index)
{
    outputBuffer[index] = 1;
    return true;
}

[numthreads(16, 1, 1)]
void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
{
    int index = dispatchThreadID.x;

    // Only the first 4 elements will be 1
    (index < 4) && assignFunc(index);

    // Only the last 4 elements will be 1.
    (index < 12) || assignFunc(index);

    //CHK-COUNT-4: 1
    //CHK-COUNT-8: 0
    //CHK-COUNT-4: 1
}
