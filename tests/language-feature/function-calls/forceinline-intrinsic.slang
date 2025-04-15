// TODO: This test fails on mtl. Enabling it only on Windows.
//TEST(windows):SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target spirv -O0 -g3
RWStructuredBuffer<int> outputBuffer;

// This tests debug information for intrinsic operations with enum conversions.
enum Direction
{
    North = 0,
    East = 1,
    South = 2,
    West = 3,
    Center = 4,
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int i = int(dispatchThreadID.x);
    
    // Perform enum conversions directly in compute shader
    // Convert from int to enum
    Direction a = Direction(i);
    
    // Convert from enum to int and perform operation
    int result = int(a) * 2;
    
    // Convert enum constant to int
    result += int(Direction.East);
    
    outputBuffer[i] = result;
}

// CHECK-COUNT-1: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugInlinedAt %uint_{{[0-9]+}} %{{[0-9]+}}
// CHECK-NOT:  %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugInlinedAt %uint_{{[0-9]+}} %{{[0-9]+}}