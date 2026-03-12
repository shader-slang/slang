// Test that structs containing arrays of ParameterBlocks are also NonAddressable
// and therefore cannot be used in arrays

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target spirv -allow-glsl -stage compute -entry computeMain
//TEST_DISABLED:SIMPLE(filecheck=CHECK): -target glsl -allow-glsl -stage compute -entry computeMain

// Test case: struct containing array of ParameterBlock
struct StructWithParameterBlockArray
{
    ParameterBlock<Texture2D> blocks[2];  // Array of ParameterBlocks
//CHECK:                      ^^^^^^ arrays of non-addressable type not allowed
    int value;
}

// This should trigger error 30028 - arrays of NonAddressable types are not allowed
StructWithParameterBlockArray myArray[3];  // Array of struct containing ParameterBlock array
//CHECK:                      ^^^^^^^ arrays of non-addressable type not allowed

[numthreads(1, 1, 1)]
void computeMain()
{
    // Empty - we're just testing the declaration
}