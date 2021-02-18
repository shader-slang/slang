//TEST:SIMPLE_LINE:-entry computeMain -target spirv -stage compute
//TEST:SIMPLE_LINE:-entry computeMain -target dxil -profile cs_6_0 
//TEST:SIMPLE_LINE:-entry computeMain -target dxbc  -stage compute
//TEST:SIMPLE_LINE:-entry computeMain -target dll -stage compute
//TEST:SIMPLE_LINE:-entry computeMain -target ptx -stage compute

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

int doThing(int a, int b) 
{
    while (b >= 0)
    {
        a 
        += 
        a;
    }
    
    return a;
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int a = dispatchThreadID.x;
    int b = dispatchThreadID.y;
    int c = dispatchThreadID.z;
    int d = a + b * c;
    int e = d + c / 2;
    
    for (int i = 0; i < b; ++i)
    {
        if (e > 10 && i & 2)
        {
            a += b; b -= c; c += c; d = d + e + a; e = a;
        }
        else
        {
            a = e; b = c + c; d += d + __SyntaxError(); e = doThing(e, dispatchThreadID.x);
            
        }
    }
    
    outputBuffer[dispatchThreadID.x] = a + b + c + d + e;
}
