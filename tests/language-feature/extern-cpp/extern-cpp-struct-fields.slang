//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type

// The fields of an `__extern_cpp` struct are emitted under their source names
// in C++ and CUDA output (https://github.com/shader-slang/slang/issues/14028).
// The emitted code for two such structs that both declare a field `value`, of
// different types, compiles and runs on each target.

__extern_cpp struct IntBox
{
    int value;
    int scale;
}

__extern_cpp struct FloatBox
{
    float value;
}

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    IntBox i;
    i.value = 7;
    i.scale = 3;

    FloatBox f;
    f.value = 2.5;

    // CHECK: 21
    outputBuffer[0] = i.value * i.scale;
    // CHECK: 5
    outputBuffer[1] = int(f.value * 2.0);
    // CHECK: 10
    outputBuffer[2] = i.value + int(f.value + 0.5);
}
