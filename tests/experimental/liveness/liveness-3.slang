//TEST:SIMPLE:-target glsl -entry computeMain -profile cs_6_3 -track-liveness -line-directive-mode none 

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

// Test loop handling, with more complex structure

int calcThing(int offset)
{
    int total = 0;
    int another[2] = { 1, 2};
    
    for (int k = 0; k < 20; ++k)
    {
        int idx[3] = {};
        
        for (int i = 0; i < 17; ++i)
        {
            int modRange = i % 3;
     
            another[i & 1] += modRange;
     
            if (i % 3 != 0)
            {
                offset += 1;
            }
            
            idx[modRange] += offset + i;
        }
        
        if ((k + 7) % 5 == 4)
        {
            return total;
        }
        
        // Now read back
        total += idx[0] + idx[1] + idx[2];
    }
    
    return -total;
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{    
    int index = int(dispatchThreadID.x);

    outputBuffer[index] = calcThing(index);
}
