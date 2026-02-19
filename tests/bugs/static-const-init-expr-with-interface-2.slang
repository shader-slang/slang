//DIAGNOSTIC_TEST:SIMPLE(diag=diag): -stage compute -entry computeMain -target spirv

interface Bug
{
    static const int badVar = 0;
//diag:              ^^^^^^ 'badVar' cannot have an initializer because it is an interface requirement
//diag:              ^^^^^^ 'badVar' cannot have an initializer because it is an interface requirement
}

RWStructuredBuffer<int> b;
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    b[0] = Bug::badVar;
}
