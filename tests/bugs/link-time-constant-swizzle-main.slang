//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry computeMain
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUFFER):-cpu -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

extern static const uint3 groupSize = uint3(8, 8, 1);

[shader("compute")]
[numthreads(8, 8, 1)]  // Use literal values for numthreads to avoid target compatibility issues
void computeMain(uint3 tid : SV_DispatchThreadID) 
{
    // Write the swizzled constant values to verify they work correctly
    uint index = tid.x;
    if (index < 3)
    {
        if (index == 0)
            outputBuffer[index] = groupSize.x; // Should be 8
        else if (index == 1)
            outputBuffer[index] = groupSize.y; // Should be 8  
        else if (index == 2)
            outputBuffer[index] = groupSize.z; // Should be 1
    }
}

// CHECK: computeMain
// BUFFER: 8
// BUFFER: 8  
// BUFFER: 1