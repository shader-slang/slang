//TEST(compute):COMPARE_COMPUTE:-cuda -output-using-type -shaderobj
//TEST(compute):COMPARE_COMPUTE:-cuda -output-using-type -shaderobj -capability cuda_sm_7_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3
//TEST(compute):COMPARE_COMPUTE:-cpu -output-using-type -shaderobj
//TEST(compute):COMPARE_COMPUTE: -output-using-type -shaderobj -render-feature double
//TEST:SIMPLE(filecheck=NVVM):-target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_7_0 -O3 -o -

// NVVM: __internal_trig_reduction_slowpathd
// NVVM: __cudart_sin_cos_coeffs
// NVVM: .visible .entry computeMain()
// NVVM-COUNT-2: st.global.f32

// Cos values are all 0 on D3d12(!)
//DISABLE_TEST(compute):COMPARE_COMPUTE: -dx12 -output-using-type -shaderobj
// When using double on vulkan the values are incorrect(!)
//DISABLE_TEST(compute,vulkan):COMPARE_COMPUTE:-vk -output-using-type -shaderobj
//DISABLE_TEST(compute):COMPARE_COMPUTE:-slang -shaderobj -mtl
// Not supported in WGSL: Double and other unsupported scalar types
//DISABLE_TEST(compute):COMPARE_COMPUTE:-slang -shaderobj -wgpu

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

float quantize(double value)
{
    return int(value * 256) * 1.0f / 256.0f;
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    float values[] = { -9, 9, -3, 3 };

    int tid = int(dispatchThreadID.x);
    float value = values[tid];

    outputBuffer[tid * 2 + 0] = quantize(sin(double(value)));
    outputBuffer[tid * 2 + 1] = quantize(cos(double(value)));
}
