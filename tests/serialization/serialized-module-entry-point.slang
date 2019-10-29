//TEST_IGNORE_FILE:

// serialized-module-entry-point.slang

struct Thing
{
    int a; 
    int b;
};

int foo(Thing thing) 
{ 
    return (thing.a + thing.b) - thing.b; 
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 ], stride=4):dxbinding(0),glbinding(0),out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    Thing thing;

    int index = (int)dispatchThreadID.x;
        
    thing.a = index;
    thing.b = -index;

    outputBuffer[index] = foo(thing);
}
