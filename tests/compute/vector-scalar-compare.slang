//TEST(compute):COMPARE_COMPUTE:-dx12 -compute -shaderobj 
//TEST(compute):COMPARE_COMPUTE:-vk -compute  -shaderobj
//TEST(compute):COMPARE_COMPUTE:-cuda -compute  -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-cuda -compute -shaderobj -capability cuda_sm_7_0 -Xslang -emit-cuda-via-nvvm
//TEST:SIMPLE(filecheck=CHECK_NVVM):-target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_7_0 -o -

// Test doing vector comparisons 

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 4, 1)]
void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
{
    int2 threadInGroup = dispatchThreadID.xy;
    
    int r = 0;
    if(all((threadInGroup & 1) == 0))
    {
        r = 0;
    }
    else
    {   
        r = 1;
    }
   
    int index = threadInGroup.x + threadInGroup.y * 4; 
    outputBuffer[index] = r;
}

// CHECK_NVVM: .visible .entry
