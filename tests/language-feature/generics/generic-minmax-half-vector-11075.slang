//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -render-features half
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -render-features half

// Regression test for shader-slang/slang#11075: `half` vectors, which need the `half` render
// feature on cuda/vk.

T genMin<T : IFloat>(T a, T b)
{
    return min(a, b);
}
T genMax<T : IFloat>(T a, T b)
{
    return max(a, b);
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    half2 mn = genMin(half2(1.5h, 9.0h), half2(4.0h, 2.5h));
    half3 mx = genMax(half3(7.5h, 1.0h, 4.0h), half3(2.0h, 6.0h, 4.0h));

    int i = 0;
    outputBuffer[i++] = float(mn.x); // CHECK: 1.5
    outputBuffer[i++] = float(mn.y); // CHECK: 2.5
    outputBuffer[i++] = float(mx.x); // CHECK: 7.5
    outputBuffer[i++] = float(mx.y); // CHECK: 6.0
    outputBuffer[i++] = float(mx.z); // CHECK: 4.0
}
