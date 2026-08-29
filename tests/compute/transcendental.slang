//TEST(compute):COMPARE_COMPUTE:-cuda -output-using-type -shaderobj
//TEST(compute):COMPARE_COMPUTE:-cuda -output-using-type -shaderobj -capability cuda_sm_7_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3
//TEST(compute):COMPARE_COMPUTE:-cpu -output-using-type -shaderobj
//TEST(compute):COMPARE_COMPUTE: -output-using-type -shaderobj
//TEST(compute,vulkan):COMPARE_COMPUTE:-vk -output-using-type -shaderobj
//TEST:SIMPLE(filecheck=NVVM):-target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_7_0 -O3 -o -

// NVVM: .visible .entry computeMain()
// NVVM-COUNT-2: st.global.f32

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

float quantize(float value)
{
    return int(value * 256) * 1.0f / 256.0f;
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    float values[] = { -9, 9, -3, 3 };

    int tid = int(dispatchThreadID.x);
    float value = values[tid];

    outputBuffer[tid * 2] = quantize(sin(value));
    outputBuffer[tid * 2 + 1] = quantize(cos(value));
}
