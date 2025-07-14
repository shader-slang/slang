//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target spirv

//CHECK: error 30623

interface Bug
{
    static const int badVar = 0;
}

RWStructuredBuffer<int> b;
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    b[0] = Bug::badVar;
}
