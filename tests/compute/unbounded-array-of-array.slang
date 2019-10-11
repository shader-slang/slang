//DISABLE_TEST:CPU_REFLECTION: -profile cs_5_0 -entry computeMain -target cpp 
//TEST(compute):COMPARE_COMPUTE_EX:-cpu -compute 

struct IntAoa { RWStructuredBuffer<int> array[]; }


//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):dxbinding(0),glbinding(0),out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

// How do I set the size of the root array?

//TEST_INPUT:array(size=2):name g_aoa.array
ParameterBlock<IntAoa> g_aoa;

//TEST_INPUT:ubuffer(data=[1 2 3 4], stride=4):name=g_aoa.array[0]
//TEST_INPUT:ubuffer(data=[8 17 34], stride=4):name=g_aoa.array[1]

[numthreads(8, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int index = int(dispatchThreadID.x);
    
    int baseIndex = index >> 2;
    int innerIndex = index & 3;

    RWStructuredBuffer<int> buffer = g_aoa.array[baseIndex];

    // Get the size 
    uint bufferCount, bufferStride;
    buffer.GetDimensions(bufferCount, bufferStride);

    if (innerIndex >= bufferCount)
    {
        innerIndex = bufferCount - 1;
    }

	outputBuffer[index] = buffer[innerIndex]; 
}