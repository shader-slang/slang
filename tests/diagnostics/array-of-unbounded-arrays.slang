// Test that arrays of unbounded arrays are not allowed

//TEST:SIMPLE(filecheck=CHECK): -target spirv -allow-glsl -stage compute -entry computeMain

// Array of unbounded arrays - should error (unbounded arrays are NonAddressable)
//CHECK: ([[# @LINE+1]]): error 30027
int arrayOfUnbounded[3][];

[numthreads(1, 1, 1)]
void computeMain()
{
    // Empty - we're just testing the declaration
}