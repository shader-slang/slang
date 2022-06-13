//TEST:SIMPLE:-target glsl -entry computeMain -profile cs_6_3 -track-liveness -line-directive-mode none

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

// Test loop handling, with more complex structure, and when code is unreachable
// More testing around liveness and loops.

int calcThing(int offset)
{    
    int arr[2] = { 2, 3};

    for (int k = 0; k < 20; ++k)
    {       
        if ((k + offset) & 1)
        {
            return arr[0];
        }
        else
        {
            return arr[1];
        }
    }
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{    
    int index = dispatchThreadID.x;

    outputBuffer[index] = calcThing(index);
}
