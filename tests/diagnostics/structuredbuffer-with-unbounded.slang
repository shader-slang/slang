// Test that StructuredBuffer of struct with unbounded array is not allowed

//TEST:SIMPLE(filecheck=CHECK): -target spirv -allow-glsl -stage compute -entry computeMain

// Struct containing unbounded array (must be last member)
struct StructWithUnbounded
{
    float value;
    int data[];  // Unbounded array must be last member
}

// StructuredBuffer of struct with unbounded array - should error
//CHECK: ([[# @LINE+1]]): error 30028
StructuredBuffer<StructWithUnbounded> myBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Empty - we're just testing the declarations
}