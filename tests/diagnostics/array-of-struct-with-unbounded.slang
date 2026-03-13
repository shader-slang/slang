// Test that arrays of structs containing unbounded arrays are not allowed

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target spirv -allow-glsl -stage compute -entry computeMain

// Struct containing unbounded array (must be last member)
struct StructWithUnbounded
{
    float value;
    int data[];  // Unbounded array must be last member
}

// Array of struct containing unbounded array - should error (struct is NonAddressable)
StructWithUnbounded myArray[2];
//CHECK:            ^^^^^^^ arrays of non-addressable type not allowed
//CHECK:            ^^^^^^^ cannot instantiate unsized type

[numthreads(1, 1, 1)]
void computeMain()
{
    // Empty - we're just testing the declarations
}