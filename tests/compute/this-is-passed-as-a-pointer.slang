//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF): -cpu
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF): -vk
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF): -vk -emit-spirv-directly -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF): -wgpu
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF): -d3d12
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF): -d3d11
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF): -metal

//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain

//TEST_INPUT: ubuffer(data=[1 2 3], stride=4):name=inputBuffer
RWStructuredBuffer<int> inputBuffer;
//TEST_INPUT: ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

struct Data
{
    int val;

    // `this` must be passed as pointer on metal/cpp/cuda.
    // `this` must be passed as `in` on glsl/hlsl
    // CUDA: (Data{{.*}}*
    int fetch(int data)
    {
        return data + val;
    }
};

//BUF: 3
//BUF: 0
//BUF: 0
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    Data data;
    data.val = inputBuffer[0];
    outputBuffer[0] = data.fetch(inputBuffer[1]);
}
