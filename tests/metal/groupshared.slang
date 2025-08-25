//TEST:SIMPLE(filecheck=CHECK): -target metal
//TEST:SIMPLE(filecheck=CHECK-ASM): -target metallib
//TEST(compute, metal):COMPARE_COMPUTE_EX(filecheck-buffer=CHK):-metal -compute -output-using-type

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
    // CHK: 0.0
    outputBuffer[0] = myArr[0];
}

// CHECK: array<int, int(16)> threadgroup* myArr{{.*}};
// CHECK: {{\[\[}}kernel{{\]\]}} void computeMain
// CHECK: threadgroup array<int, int(16)> myArr{{.*}};
// CHECK: (&kernelContext{{.*}})->myArr{{.*}} = &myArr{{.*}};
// CHECK-ASM: define void @computeMain

[numthreads(1,1,1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    myArr[tid.x] = tid.x;
    func(3.0f);
}
