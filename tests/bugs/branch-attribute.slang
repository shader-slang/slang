//TEST:SIMPLE(filecheck=HLSL): -target hlsl -profile cs_5_0 -entry computeMain
//TEST:SIMPLE(filecheck=SPIRV): -target spirv -O0

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

// HLSL: [branch]
// HLSL: [flatten]
// SPIRV: OpSelectionMerge {{.*}} DontFlatten
// SPIRV: OpSelectionMerge {{.*}} Flatten

[numthreads(4, 1, 1)]
void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
{
    int idx = dispatchThreadID.x;
    
    int v = idx;
    
    [branch]
    if (idx != 0)
    {
        ++v;
    }
    
    [flatten]
    if (idx > 2)
    {
        v += v;
    }

	outputBuffer[dispatchThreadID.x] = v;
}