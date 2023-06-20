// pointer-self-reference.slang

//TEST(compute):SIMPLE:-target cpp -entry computeMain -stage compute

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

struct Thing
{
    int value;
    Ptr<Thing> next;
};

[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
    int idx = dispatchThreadID.x;
    
    Thing things[2];
    
    things[0].next = &things[1];
    things[0].value = 27;
    
    things[1].next = &things[0];
    things[1].value = idx * idx;
    
    Ptr<Thing> cur = &things[0];
    
    for (int i = 0; i < idx; ++i)
    {
        cur = cur.next;
    }
    
    int v = cur.value;
    
    outputBuffer[idx] = v; 
}
