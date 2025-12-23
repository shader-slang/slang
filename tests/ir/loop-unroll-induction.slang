//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -output-using-type -xslang -g0
//DISABLED_TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -llvm -output-using-type -xslang -g0
//TEST_INPUT:set output = out ubuffer(data=[0], stride=4)

RWStructuredBuffer<int> output;

int calcOffset(uint rank)
{
    uint offset = 0;
    [ForceUnroll]
    for (int i = 0; i < 3; i++)
    {
        if (i >= 2)
            break;
        offset += i;
    }
    return offset;
}

[numthreads(1,1,1)]
void computeMain()
{
    // CHECK: 1
    output[0] = calcOffset(2);
}
