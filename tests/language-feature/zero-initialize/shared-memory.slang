//TEST:SIMPLE(filecheck=HLSL): -target hlsl -stage compute -entry computeMain -zero-initialize
//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage compute -entry computeMain -zero-initialize

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

// GLSL-NOT: globalMem{{.*}} =
// HLSL-NOT: globalMem{{.*}} =

groupshared uint globalMem;

[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
// BUF: 1
    outputBuffer[0] = globalMem ;
}
