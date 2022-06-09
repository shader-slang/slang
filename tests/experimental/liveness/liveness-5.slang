//TEST:SIMPLE:-target glsl -entry computeMain -profile cs_6_3 -track-liveness -line-directive-mode none 

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

// Test loop handling, with more complex structure
// Here to test anothers liveness.
// NOTE! That another is load/stored but it's value is never used externally.
// which leads to liveness ending in the loop, which seems odd and means another can contain 'rubbish'
// but arguably that doesn't matter because it is not read from.
//
// This can lead for *multiple* livenessEnd through an execution path that can be hit.

int calcThing(int offset)
{
    int total = 0;
    int another[2] = { 1, 2};
    
    for (int k = 0; k < 20; ++k)
    {       
        for (int i = 0; i < 17; ++i)
        {
            another[i & 1] += k + i;
        }
        
        total += another[k & 1];
        
        if ((k + 7) % 5 == 4)
        {
            return 1;
        }
    }
    
    if (total > 4)
    {
        total = -total;
    }
    
    return total;
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{    
    int index = dispatchThreadID.x;

    outputBuffer[index] = calcThing(index);
}
