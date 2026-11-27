//TEST:COMPARE_COMPUTE(targets=all-glsl-dx11-dx12-wgpu, filecheck-buffer=CHECK): -output-using-type

//TEST_INPUT: set ptr = ubuffer(data=[42], stride=4)
uniform int* ptr;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    // should load value into val.
    // There was a bug where the type deduced for `val` was `Ref<int>` instead of `int`,
    // causing the value to be read from `val` being incorrect.
    let val = *ptr;
    *ptr = 100;
    let val2 = *ptr;
    // CHECK: 42
    outputBuffer[0] = val;
    // CHECK: 100
    outputBuffer[1] = val2;
}