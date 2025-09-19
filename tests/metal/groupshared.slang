//TEST:SIMPLE(filecheck=METAL): -target metal
//TEST:SIMPLE(filecheck=METALLIB): -target metallib
//TEST(compute, metal):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-metal -compute -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -output-using-type

//TEST_INPUT: ubuffer(data=[2.0], stride=1):out,name outputBuffer
uniform RWStructuredBuffer<float> outputBuffer;

struct MyBlock
{
    StructuredBuffer<float> b1;
    StructuredBuffer<float> b2;
}
ParameterBlock<MyBlock> block;

groupshared int myArr[16];

void func(float v)
{
    // BUF: 0.000000
    outputBuffer[0] = myArr[0];
}

// METAL: array<int, int(16)> threadgroup* myArr{{.*}};
// METAL: {{\[\[}}kernel{{\]\]}} void computeMain
// METAL: threadgroup array<int, int(16)> myArr{{.*}};
// METAL: (&kernelContext{{.*}})->myArr{{.*}} = &myArr{{.*}};
// METALLIB: define void @computeMain

[numthreads(1,1,1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    myArr[tid.x] = tid.x;
    func(3.0f);
}
