//TEST:COMPILE: tests/bugs/link-time-constant-swizzle-lib.slang -o tests/bugs/link-time-constant-swizzle-lib.slang-module
//TEST:COMPILE: tests/bugs/link-time-constant-swizzle-main.slang -o tests/bugs/link-time-constant-swizzle-main.slang-module
//TEST:SIMPLE(filecheck=CHECK): -r tests/bugs/link-time-constant-swizzle-main.slang-module -r tests/bugs/link-time-constant-swizzle-lib.slang-module -target hlsl -stage compute -entry computeMain

extern static const uint groupSizeX;
extern static const uint groupSizeY;
extern static const uint groupSizeZ;
extern static const uint3 groupSize;

RWStructuredBuffer<uint> outputBuffer;

[shader("compute")]
[numthreads(groupSizeX, groupSizeY, groupSizeZ)]  // Individual extern constants work
void computeMain(uint3 tid : SV_DispatchThreadID) 
{
    // Test swizzled constants in array size - this is the main fix we're testing
    uint testArray[groupSize.x];  // This should work with our SwizzleExpr fix
    uint testArray2[groupSize.y];
    uint testArray3[groupSize.z];
    
    // Atomically increment to count how many threads were launched
    // Expected: 8 * 8 * 1 = 64 threads total
    uint originalValue;
    InterlockedAdd(outputBuffer[0], 1, originalValue);
}

// CHECK: computeMain