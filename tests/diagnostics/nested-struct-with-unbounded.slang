// Test that arrays and StructuredBuffers of nested structs with unbounded arrays are not allowed

//TEST:SIMPLE(filecheck=CHECK): -target spirv -allow-glsl -stage compute -entry computeMain

// Struct containing unbounded array (must be last member)
struct StructWithUnbounded
{
    float value;
    int data[];  // Unbounded array must be last member
}

// Nested case
struct NestedUnbounded
{
    int id;
    StructWithUnbounded nested;  // Struct with unbounded array member
}

// Array of nested struct - should error
//CHECK: ([[# @LINE+1]]): error 30027
NestedUnbounded nestedArray[5];

// StructuredBuffer of nested struct - should error
//CHECK: ([[# @LINE+1]]): error 30028
StructuredBuffer<NestedUnbounded> nestedBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Empty - we're just testing the declarations
}