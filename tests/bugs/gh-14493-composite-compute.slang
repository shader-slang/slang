// Search results have an undefined incoming before a match. A fallback makes
// every observed component defined. The carried aggregate and partial array
// updates intentionally depend on previous iterations and must remain intact.

//TEST:COMPARE_COMPUTE(targets=vk+cpu, filecheck-buffer=BUF): -entry computeMain -output-using-type
//TEST:COMPARE_COMPUTE(targets=vk, filecheck-buffer=BUF):-entry computeMain -output-using-type -xslang -minimum-slang-optimization

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0 0.5 8.0 1.5 8.0], stride=4):name=inputBuf
StructuredBuffer<float> inputBuf;
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outBuf
RWStructuredBuffer<float> outBuf;

struct SearchResult
{
    float2 pair;
    float weights[2];
    float2x2 basis;
};

SearchResult makeResult(float value)
{
    return {
        float2(value, value + 1),
        {value + 2, value + 3},
        float2x2(value + 4, value + 5, value + 6, value + 7)};
}

[ForceInline]
SearchResult find(float threshold)
{
    SearchResult result;
    bool found = false;
    for (uint index = 0; index < 4; ++index)
    {
        float value = inputBuf[index];
        if (threshold < value)
        {
            result = makeResult(value);
            found = true;
            break;
        }
    }
    if (!found)
        result = {float2(-1, -2), {-3, -4}, float2x2(-5, -6, -7, -8)};
    return result;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    SearchResult carried = {float2(10, 20), {30, 40}, float2x2(50, 60, 70, 80)};
    float partial[2] = {3, 4};
    for (uint iteration = 0; iteration < 4; ++iteration)
    {
        SearchResult result = find(inputBuf[4 + iteration]);
        carried = {
            carried.pair + result.pair,
            {carried.weights[0] + result.weights[1], carried.weights[0]},
            float2x2(carried.basis[0][0] + result.basis[1][1], 60, 70, 80)};
        // A partial store must preserve the other component across iterations.
        partial[iteration & 1] += result.pair.x;
        outBuf[iteration * 9] = result.pair.x;
        outBuf[iteration * 9 + 1] = result.weights[1];
        outBuf[iteration * 9 + 2] = result.basis[1][1];
        outBuf[iteration * 9 + 3] = carried.pair.x;
        outBuf[iteration * 9 + 4] = carried.weights[0];
        outBuf[iteration * 9 + 5] = carried.weights[1];
        outBuf[iteration * 9 + 6] = carried.basis[0][0];
        // Dynamic extraction takes a snapshot of the selected aggregate value.
        outBuf[iteration * 9 + 7] = result.weights[iteration & 1];
        outBuf[iteration * 9 + 8] = result.basis[iteration & 1][1];
    }
    outBuf[36] = partial[0];
    outBuf[37] = partial[1];
}

// BUF: 1.0
// BUF-NEXT: 4.0
// BUF-NEXT: 8.0
// BUF-NEXT: 11.0
// BUF-NEXT: 34.0
// BUF-NEXT: 30.0
// BUF-NEXT: 58.0
// BUF-NEXT: 3.0
// BUF-NEXT: 6.0
// BUF-NEXT: -1.0
// BUF-NEXT: -4.0
// BUF-NEXT: -8.0
// BUF-NEXT: 10.0
// BUF-NEXT: 30.0
// BUF-NEXT: 34.0
// BUF-NEXT: 50.0
// BUF-NEXT: -4.0
// BUF-NEXT: -8.0
// BUF-NEXT: 2.0
// BUF-NEXT: 5.0
// BUF-NEXT: 9.0
// BUF-NEXT: 12.0
// BUF-NEXT: 35.0
// BUF-NEXT: 30.0
// BUF-NEXT: 59.0
// BUF-NEXT: 4.0
// BUF-NEXT: 7.0
// BUF-NEXT: -1.0
// BUF-NEXT: -4.0
// BUF-NEXT: -8.0
// BUF-NEXT: 11.0
// BUF-NEXT: 31.0
// BUF-NEXT: 35.0
// BUF-NEXT: 51.0
// BUF-NEXT: -4.0
// BUF-NEXT: -8.0
// BUF-NEXT: 6.0
// BUF-NEXT: 2.0
