// TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF): -cuda
// TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF): -cpu
// TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF): -vk -emit-spirv-via-glsl
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF): -vk -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF): -wgpu
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF): -d3d12
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF): -d3d11
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF): -slang -compute -mtl -render-features argument-buffer-tier-2

//TEST:SIMPLE(filecheck=METALLIB): -target metallib -entry computeMain
//METALLIB: @computeMain

struct Data {
    RWStructuredBuffer<int> input;
    RWStructuredBuffer<int> output;
    int val;
};

int fetchInner(Data thisData, int index)
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

// BUF: C
    data.output[0] = res;
}
