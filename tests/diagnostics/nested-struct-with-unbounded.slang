// Test that arrays and StructuredBuffers of nested structs with unbounded arrays are not allowed

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target spirv -allow-glsl -stage compute -entry computeMain
// Using non-exhaustive because there are many cascading duplicate errors

// Struct containing unbounded array (must be last member)
struct StructWithUnbounded
{
    float value;
    int data[];  // Unbounded array must be last member
}

// Nested case
struct NestedUnbounded
//CHECK: function parameter cannot be unsized
{
    int id;
    StructWithUnbounded nested;  // Struct with unbounded array member
}

// Array of nested struct - should error
NestedUnbounded nestedArray[5];
//CHECK: arrays of non-addressable type not allowed

// StructuredBuffer of nested struct - should error
StructuredBuffer<NestedUnbounded> nestedBuffer;
//CHECK: non-addressable type cannot be used in StructuredBuffer

[numthreads(1, 1, 1)]
void computeMain()
{
    // Empty - we're just testing the declarations
}