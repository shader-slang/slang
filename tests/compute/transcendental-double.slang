//TEST:COMPARE_COMPUTE(targets=cuda+cpu):-output-using-type
// TODO: Remove `glsl` from the targets when https://github.com/shader-slang/slang/issues/14589 is fixed. glslang rejects `sin` of a
// double.
//TEST:COMPARE_COMPUTE(targets=all-vk-glsl-dx12-metal-wgpu): -output-using-type -render-feature double

// Cos values are all 0 on D3d12(!)
// When using double on vulkan the values are incorrect(!)
// Not supported in WGSL: Double and other unsupported scalar types

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
