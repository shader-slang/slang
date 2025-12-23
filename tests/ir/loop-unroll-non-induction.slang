//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -output-using-type -xslang -g0

//TEST_INPUT:set output = out ubuffer(data=[0 0], stride=4)

RWStructuredBuffer<int> output;

int calcOffset(uint rank)
{
    uint offset = 1;
    [ForceUnroll]
    for (int i = 0; i < 3; i++)
    {
        if (i >= rank)
            break;
        offset = i*2;
    }
    return offset;
}

[numthreads(1,1,1)]
void computeMain()
{
    // CHECK: 2
    output[0] = calcOffset(2);
    // CHECK: 1
    output[1] = calcOffset(0);
}
