// Test that unbounded arrays are NonAddressable and cannot be used in arrays

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target spirv -allow-glsl -stage compute -entry computeMain
// Using non-exhaustive due to multiple duplicate cascading errors

// Unbounded array itself is fine
int unboundedArray[];

// Array of unbounded arrays - should error (unbounded arrays are NonAddressable)
int arrayOfUnbounded[3][];
//CHECK: arrays of non-addressable type not allowed

// Struct containing unbounded array (must be last member)
struct StructWithUnbounded
{
    float value;
    int data[];  // Unbounded array must be last member
}

// Array of struct containing unbounded array - should error (struct is NonAddressable)
StructWithUnbounded myArray[2];
//CHECK: arrays of non-addressable type not allowed

// StructuredBuffer of struct with unbounded array - should error
StructuredBuffer<StructWithUnbounded> myBuffer;
//CHECK: non-addressable type cannot be used in StructuredBuffer

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