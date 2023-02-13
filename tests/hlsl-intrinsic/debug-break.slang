//TEST:SIMPLE:-stage compute -entry computeMain -target glsl -line-directive-mode none
// We can't enable because output source includes path to prelude.
//DISABLE_TEST:SIMPLE:-stage compute -entry computeMain -target cpp -line-directive-mode none
//DISABLE_TEST:SIMPLE:-stage compute -entry computeMain -target cuda -line-directive-mode none

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer

RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    const int idx = int(dispatchThreadID.x);

    debugBreak();

    outputBuffer[idx] = idx;    
}