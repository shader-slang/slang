//TEST:SIMPLE(filecheck=SPIRV): -target spirv -stage compute -entry main -O2

StructuredBuffer<bfloat16_t> inputBuffer;
RWStructuredBuffer<bfloat16_t> outputBuffer;

bfloat16_t func(bfloat16_t val)
{
    if (float(val) >= 0)
    {
        //SPIRV: OpSelect %bfloat16
        return float(val) == 0 ? val : 1.bf;
    }
    //SPIRV: OpPhi %bfloat16
    return -1.bf;
}

[numthreads(1, 1, 1)]
void main(int3 dispatchThreadID : SV_DispatchThreadID)
{
    const int idx = dispatchThreadID.x;
    outputBuffer[idx * 2 + 0] = func(inputBuffer[idx]);
}
