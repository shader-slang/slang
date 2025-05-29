//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -output-using-type
//TEST(compute,vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -slang -compute -output-using-type

[Differentiable]
Optional<float> sumSquare(Optional<float> a, Optional<float> b)
{
    if (let x = a)
    {
        if (let y = b)
        {
            return x * x + y * y;
        }
        else
        {
            return x * x;
        }
    }
    else if (let y = b)
    {
        return y * y;
    }
    return none;
}

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    var dpa = diffPair<Optional<float>>(3.0f, none);
    var dpb = diffPair<Optional<float>>(4.0f, none);

    bwd_diff(sumSquare)(dpa, dpb, 1.0f);

    outputBuffer[0] = -1;

    // CHECK: 14.0
    if (dpa.d.hasValue && dpb.d.hasValue)
        outputBuffer[0] = dpa.d.value + dpb.d.value;

    // CHECK: 1.0
    dpa = diffPair<Optional<float>>(3.0f, none);
    dpb = diffPair<Optional<float>>(4.0f, none);
    bwd_diff(sumSquare)(dpa, dpb, none);
    if (dpa.d.value == 0.0 && dpb.d.value == 0.0)
    {
        outputBuffer[1] = 1.0f;
    }
}