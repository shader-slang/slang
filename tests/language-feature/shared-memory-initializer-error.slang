//DIAGNOSTIC_TEST:SIMPLE(diag=diag): -target hlsl -stage compute -entry computeMain
//DIAGNOSTIC_TEST:SIMPLE(diag=diag): -target glsl -stage compute -entry computeMain

RWStructuredBuffer<uint> outputBuffer;

groupshared uint globalMem = 1;
//diag:          ^^^^^^^^^ 'globalMem' cannot have an initializer because it is groupshared
//diag:          ^^^^^^^^^ 'globalMem' cannot have an initializer because it is groupshared

[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
    outputBuffer[0] = globalMem ;
}
