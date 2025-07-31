//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF): -cuda
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF): -cpu
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF): -vk
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF): -vk -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF): -wgpu
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF): -d3d12
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF): -d3d11
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF): -slang -compute -mtl -render-features argument-buffer-tier-2


//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain
//TEST:SIMPLE(filecheck=CPP): -target cpp -entry computeMain
//TEST:SIMPLE(filecheck=METAL): -target metal -entry computeMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -entry computeMain

// CUDA: fetch{{.*}}Data_0{{.*}}* this
// CPP: fetch{{.*}}Data_0{{.*}}* this
// METAL: fetch{{.*}}Data_{{.*}} thread* this
// METALLIB: @computeMain

struct Data {
    RWStructuredBuffer<int> input;
    RWStructuredBuffer<int> output;
    int val;

    [constref]
    int fetch(int index)
    {
        return input.Load(index) + val;
    }
};

//TEST_INPUT:set data = new Data{ubuffer(data=[1 2 3 4], stride = 4), out ubuffer(data=[0 0 0 0], stride = 4), 10}
ParameterBlock<Data> data;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    let res = data.fetch(data.input[0]);

//BUF: C
    data.output[0] = res;
}
