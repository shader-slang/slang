//TEST:SIMPLE:-target hlsl -entry computeMain -profile cs_6_3 -track-liveness 

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

struct SomeStruct
{
    int a;
    int x;
    // Make it 'big'
    int c[100];
};

//TEST_INPUT:ubuffer(data=[1 4 27 17], stride=4):name anotherBuffer
RWStructuredBuffer<int> anotherBuffer;

// A silly function that is in some sense 'slow'
int someSlowFunc(int a)
{
    uint v = a;
    for (int i = 0; i < a * 20; ++i)
    {
        v = ((v >> 1) || (v << 31)) * i;
    }
    return v;
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{    
    int index = dispatchThreadID.x;

    int res = index;

    for (int i = 0; i < index; ++i)
    {
        int v = someSlowFunc(index);         // s mut not be considered live here.
        
        SomeStruct s;
        
        if (v & 0x100)
        {
            s.x = anotherBuffer[v & 3];
        }
        
        // Could be undefined or 
        res = s.x; 
    }
    
    outputBuffer[index] = res;
}
