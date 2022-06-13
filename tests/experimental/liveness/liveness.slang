//TEST:SIMPLE:-target glsl -entry computeMain -profile cs_6_3 -track-liveness -line-directive-mode none 

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

SomeStruct makeSomeStruct()
{
    SomeStruct s = {};
    return s;
}

// A silly function that is in some sense 'slow'
int someSlowFunc(int a)
{
    uint v = a;
    for (int i = 0; i < a * 20; ++i)
    {
        v = ((v >> 1) | (v << 31)) * i;
    }
    return v;
}

int somethingElse(inout SomeStruct s)
{
    s.x ++;
    return s.x;
}

int doThing(SomeStruct s)
{
    return s.x * 2 + 1;
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
        SomeStruct t = makeSomeStruct();
        SomeStruct u = {};
        
        if (v & 0x100)
        {
            s.x = anotherBuffer[v & 3];
            t.x = anotherBuffer[v & 3];
        }
        else
        {
            {
                SomeStruct x = u;
                x.x = anotherBuffer[v & 3] + 1;
                u = x;
            }
        }
        
        s.c[index & 7]++;
        
        res += s.x + t.x + u.x + doThing(t) + somethingElse(t) + s.c[2];
    }
    
    outputBuffer[index] = res;
}
