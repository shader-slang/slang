//TEST:SIMPLE(filecheck=COMPILE): -target hlsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=COMPILE): -target spirv -stage compute -entry computeMain

// A `.hlsl` file extension selects the HLSL-flavored dialect on its own, which is
// the remedy the declaration-as-condition diagnostic advertises, so a declaration
// condition must compile here without any explicit `-lang`/`-source-language` flag.

// COMPILE: result code = 0

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int c = 3;
    if (int count = c)
        outputBuffer[0] = count;
    else
        outputBuffer[0] = 0;
}
