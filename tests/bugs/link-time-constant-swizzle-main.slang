//TEST:COMPILE: tests/bugs/link-time-constant-swizzle-lib.slang -o tests/bugs/link-time-constant-swizzle-lib.slang-module
//TEST:COMPILE: tests/bugs/link-time-constant-swizzle-main.slang -o tests/bugs/link-time-constant-swizzle-main.slang-module
//TEST:SIMPLE(filecheck=CHECK): -r tests/bugs/link-time-constant-swizzle-main.slang-module -r tests/bugs/link-time-constant-swizzle-lib.slang-module -target hlsl -stage compute -entry computeMain

extern static const uint groupSizeX;
extern static const uint groupSizeY;
extern static const uint groupSizeZ;
extern static const uint3 groupSize;

[shader("compute")]
[numthreads(groupSizeX, groupSizeY, groupSizeZ)]  // This works - individual extern const
void computeMain(uint3 tid : SV_DispatchThreadID) 
{
    // Use swizzled constants in array size - this is what we're testing
    uint testArray[groupSize.x];  // This should work with our fix
}

// CHECK: computeMain