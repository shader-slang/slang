// Test that unbounded arrays are NonAddressable and cannot be used in arrays

//TEST:SIMPLE(filecheck=CHECK): -target spirv -allow-glsl -stage compute -entry computeMain

// Unbounded array itself is fine
int unboundedArray[];

// Array of unbounded arrays - should error (unbounded arrays are NonAddressable)
//CHECK: ([[# @LINE+1]]): error 30027
int arrayOfUnbounded[3][];

// Struct containing unbounded array (must be last member)
struct StructWithUnbounded
{
    float value;
    int data[];  // Unbounded array must be last member
}

// Array of struct containing unbounded array - should error (struct is NonAddressable)
//CHECK: ([[# @LINE+1]]): error 30027
StructWithUnbounded myArray[2];

// StructuredBuffer of struct with unbounded array - should error
//CHECK: ([[# @LINE+1]]): error 30028
StructuredBuffer<StructWithUnbounded> myBuffer;

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