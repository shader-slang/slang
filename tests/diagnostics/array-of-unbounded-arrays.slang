// Test that arrays of unbounded arrays are not allowed

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target spirv -allow-glsl -stage compute -entry computeMain

// Array of unbounded arrays - should error (unbounded arrays are NonAddressable)
int arrayOfUnbounded[3][];
/*CHECK:
    ^^^^^^^^^^^^^^^^ arrays of non-addressable type not allowed
    ^^^^^^^^^^^^^^^^ cannot instantiate unsized type
*/

[numthreads(1, 1, 1)]
void computeMain()
{
    // Empty - we're just testing the declaration
}