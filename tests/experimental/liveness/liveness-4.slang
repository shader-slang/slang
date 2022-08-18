//TEST:SIMPLE:-target glsl -entry computeMain -profile cs_6_3 -track-liveness -line-directive-mode none 

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

// Test loop handling, with more complex structure
// Here to test anothers liveness.

int calcThing(int offset)
{
    int another[2] = { 1, 2};
    
    for (int k = 0; k < 20; ++k)
    {       
        for (int i = 0; i < 17; ++i)
        {
            another[i & 1] += k + i;
        }
        
        if ((k + 7) % 5 == 4)
        {
            return 1;
        }
    }
    
    return -2;
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{    
    int index = int(dispatchThreadID.x);

    outputBuffer[index] = calcThing(index);
}
