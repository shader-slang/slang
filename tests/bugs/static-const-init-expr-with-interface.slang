//TEST:SIMPLE(filecheck=ERROR_CHECK): -stage compute -entry computeMain -target spirv
//TEST:LANG_SERVER(filecheck=CHECK):

//ERROR_CHECK: error 30623

//HOVER:7,25
//CHECK:Bug.badVar = 0

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
