// Std430 remains unavailable to actual CUDA source output, including minimum optimization.
//TEST:SIMPLE(filecheck=BAD): -target cuda -entry computeMain -stage compute -capability cuda_sm_8_0 -O0 -o std430-cuda-o0.cu
//TEST:SIMPLE(filecheck=BAD): -target cuda -entry computeMain -stage compute -capability cuda_sm_8_0 -O3 -o std430-cuda-o3.cu
//TEST:SIMPLE(filecheck=BAD): -target cuda -entry computeMain -stage compute -capability cuda_sm_8_0 -minimum-slang-optimization -o std430-cuda-min.cu
//TEST:SIMPLE(filecheck=BAD): -target cuda -entry computeBuffer -stage compute -capability cuda_sm_8_0 -O0 -o std430-buffer-o0.cu
//TEST:SIMPLE(filecheck=BAD): -target cuda -entry computeBuffer -stage compute -capability cuda_sm_8_0 -O3 -o std430-buffer-o3.cu
//TEST:SIMPLE(filecheck=BAD): -target cuda -entry computeBuffer -stage compute -capability cuda_sm_8_0 -minimum-slang-optimization -o std430-buffer-min.cu
// BAD: result code = {{-?[1-9][0-9]*}}
// BAD: error[E55200]

struct Record
{
    float3 value;
    bool flag;
};

[CUDAKernel]
void computeMain(
    uniform LayoutPtr<Record, Std430DataLayout> input,
    uniform int index,
    uniform Ptr<uint64_t> output)
{
    output[0] = uint64_t(input + index);
}

StructuredBuffer<Record, Std430DataLayout> inputBuffer;
RWStructuredBuffer<float> outputBuffer;
[numthreads(1,1,1)]
void computeBuffer()
{
    outputBuffer[0] = inputBuffer[0].value.x;
}
