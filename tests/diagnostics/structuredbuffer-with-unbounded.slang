// Test that StructuredBuffer of struct with unbounded array is not allowed

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -allow-glsl -stage compute -entry computeMain

// Struct containing unbounded array (must be last member)
struct StructWithUnbounded
{
    float value;
    int data[];  // Unbounded array must be last member
}

// StructuredBuffer of struct with unbounded array - should error
StructuredBuffer<StructWithUnbounded> myBuffer;
//CHECK:                              ^^^^^^^^ non-addressable type cannot be used in StructuredBuffer
//CHECK:                              ^^^^^^^^ 'StructWithUnbounded' is non-addressable and cannot be used in StructuredBuffer

[numthreads(1, 1, 1)]
void computeMain()
{
    // Empty - we're just testing the declarations
}