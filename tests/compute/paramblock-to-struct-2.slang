//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF): -cuda 
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF): -cpu
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF): -vk
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF): -vk -emit-spirv-directly
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF): -wgpu
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF): -d3d12
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF): -d3d11
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF): -metal

struct Data {
    RWStructuredBuffer<int> input;
    RWStructuredBuffer<int> output;
    int val;  
};

int fetchInner(__constref Data thisData, int index)
{
    return thisData.input.Load(index) + thisData.val;
}

int fetch(ParameterBlock<Data> thisData, int index)
{
    return fetchInner(thisData, index);
}

//TEST_INPUT:set data = new Data{ubuffer(data=[1 2 3 4], stride = 4), out ubuffer(data=[0 0 0 0], stride = 4), 10}
ParameterBlock<Data> data;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    let res = fetch(data, data.input[0]);

//BUF: C
    data.output[0] = res;
}
