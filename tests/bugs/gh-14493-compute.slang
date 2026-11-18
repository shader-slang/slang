// Exercise the inlined search from #14493 with both a match and an exhausted
// search on successive iterations. The undefined incoming of the search's
// internal result must not affect the fallback. The accumulator and swapped
// vector, in contrast, intentionally carry values between iterations.

//TEST:COMPARE_COMPUTE(targets=vk+cpu, filecheck-buffer=BUF): -entry computeMain -output-using-type
//TEST:COMPARE_COMPUTE(targets=vk, filecheck-buffer=BUF):-entry computeMain -output-using-type -xslang -minimum-slang-optimization

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0 0.5 8.0 1.5 8.0], stride=4):name=inputBuf
StructuredBuffer<float> inputBuf;
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outBuf
RWStructuredBuffer<float> outBuf;

[ForceInline]
float find(float u)
{
    for (uint i = 0; i < 4; ++i)
        if (u < inputBuf[i])
            return inputBuf[i];
    return -1.0;
}

[ForceInline]
float2 findVec(float u)
{
    for (uint i = 0; i < 4; ++i)
        if (u < inputBuf[i])
            return float2(inputBuf[i], inputBuf[i] + 1.0);
    return float2(-1.0, -2.0);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    float total = 10.0;
    float2 swapped = float2(1.0, 2.0);
    for (uint k = 0; k < 4; ++k)
    {
        float u = inputBuf[4 + k];
        float scalar = find(u);
        float2 shaped = findVec(u);
        total += scalar;
        swapped = swapped.yx;
        outBuf[k * 4] = scalar;
        outBuf[k * 4 + 1] = shaped.x;
        outBuf[k * 4 + 2] = shaped.y;
        outBuf[k * 4 + 3] = total;
    }
    outBuf[16] = swapped.x;
    outBuf[17] = swapped.y;
}

// BUF: 1.0
// BUF-NEXT: 1.0
// BUF-NEXT: 2.0
// BUF-NEXT: 11.0
// BUF-NEXT: -1.0
// BUF-NEXT: -1.0
// BUF-NEXT: -2.0
// BUF-NEXT: 10.0
// BUF-NEXT: 2.0
// BUF-NEXT: 2.0
// BUF-NEXT: 3.0
// BUF-NEXT: 12.0
// BUF-NEXT: -1.0
// BUF-NEXT: -1.0
// BUF-NEXT: -2.0
// BUF-NEXT: 11.0
// BUF-NEXT: 1.0
// BUF-NEXT: 2.0
