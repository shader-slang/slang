//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF): -cuda
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF): -cpu
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF): -vk
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF): -vk -emit-spirv-directly
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF): -d3d12
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF): -d3d11
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF): -metal
// WGSL bug (Array of resource types can generate an array<array<T>,U>, which is illegal WGSL), tracked with issue #7980
//DISABLE_TEST:COMPARE_COMPUTE(filecheck-buffer=BUF): -wgpu

//TEST:SIMPLE(filecheck=METALLIB): -target metallib -entry computeMain

// METALLIB: @computeMain

struct Data {
    RWStructuredBuffer<int> input[2];
    RWStructuredBuffer<int> output[2];
    int val;

    int fetch(int index)
    {
        return input[1].Load(index) + val;
    }
};

//TEST_INPUT:set data = new Data{{ubuffer(data=[1 1 1 1], stride = 4), ubuffer(data=[2 2 2 2], stride = 4)}, {out ubuffer(data=[0 0 0 0], stride = 4), ubuffer(data=[0 0 0 0], stride = 4)}, 10}
ParameterBlock<Data> data;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    let res = data.fetch(data.input[0][0]);

//BUF: C
    data.output[0][0] = res;
}
