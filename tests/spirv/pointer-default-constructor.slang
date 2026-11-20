//TEST:COMPARE_COMPUTE(targets=all-glsl-dx11-dx12-wgpu, filecheck-buffer=CHECK):-output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

struct MyStruct
{
    int* ptr;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    float* a = {};
    MyStruct ms = {};

    // CHECK: 0
    outputBuffer[0] = uint(uint64_t(a));
    // CHECK: 1
    outputBuffer[1] = uint(a == nullptr);
    // CHECK: 0
    outputBuffer[2] = uint(uint64_t(ms.ptr));
    // CHECK: 1
    outputBuffer[3] = uint(ms.ptr == nullptr);
}
