// Companion module for `shader-64bit-indexing-precompiled.slang`, compiled to a `.slang-module`.

RWStructuredBuffer<int> outputBuffer;

[Shader64BitIndexing]
int helper(int n)
{
    return n + 1;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int i = dispatchThreadID.x;
    outputBuffer[i] = helper(outputBuffer[i]);
}
