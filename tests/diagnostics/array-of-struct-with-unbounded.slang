// Test that arrays of structs containing unbounded arrays are not allowed

//TEST:SIMPLE(filecheck=CHECK): -target spirv -allow-glsl -stage compute -entry computeMain

// Struct containing unbounded array (must be last member)
struct StructWithUnbounded
{
    float value;
    int data[];  // Unbounded array must be last member
}

// Array of struct containing unbounded array - should error (struct is NonAddressable)
//CHECK: ([[# @LINE+1]]): error 30027
StructWithUnbounded myArray[2];

[numthreads(1, 1, 1)]
void computeMain()
{
    // Empty - we're just testing the declarations
}