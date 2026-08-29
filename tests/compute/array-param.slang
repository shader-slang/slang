//TEST(compute):COMPARE_COMPUTE_EX:-cuda -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-cuda -compute -shaderobj -capability cuda_sm_7_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX:-cpu -compute -compile-arg -O3 -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -dx12 -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj
//TEST:SIMPLE(filecheck=NVVM):-target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_7_0 -O3 -o -

// NVVM: .visible .entry computeMain()
// NVVM: st.global.u32


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
