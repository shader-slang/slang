//TEST:SIMPLE(filecheck=HLSL): -target hlsl -profile cs_5_0 -entry computeMain
//TEST:SIMPLE(filecheck=SPIRV): -target spirv -O0

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

// HLSL: [branch]
// SPIRV: OpSelectionMerge {{.*}} DontFlatten

[numthreads(4, 1, 1)]
void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
{
    int idx = dispatchThreadID.x;
    
    int v = idx;
   
   [branch]
    switch (idx)
    {
        case 0: v ++; break;
        case 2: v -= 7; break;
        default: v--;
    }

	outputBuffer[dispatchThreadID.x] = v;
}