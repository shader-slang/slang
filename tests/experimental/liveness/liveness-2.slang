//TEST:SIMPLE:-target glsl -entry computeMain -profile cs_6_3 -track-liveness -line-directive-mode none 

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

// Test loop handling.

int calcThing(int offset)
{
    int idx[3];
    
    for (int i = 0; i < 3; ++i)
    {
        idx[i] = offset + i;
    }
    
    // Now read back
    return idx[0] + idx[1] + idx[2];
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{    
    int index = dispatchThreadID.x;

    outputBuffer[index] = calcThing(index);
}
