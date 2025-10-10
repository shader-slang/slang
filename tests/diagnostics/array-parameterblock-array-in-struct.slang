// Test that structs containing arrays of ParameterBlocks are also NonAddressable
// and therefore cannot be used in arrays

//TEST:SIMPLE(filecheck=CHECK): -target spirv -allow-glsl -stage compute -entry computeMain
//TEST_DISABLED:SIMPLE(filecheck=CHECK): -target glsl -allow-glsl -stage compute -entry computeMain  

// Test case: struct containing array of ParameterBlock
struct StructWithParameterBlockArray
{
    ParameterBlock<Texture2D> blocks[2];  // Array of ParameterBlocks
    int value;
}

// This should trigger error 30027 - arrays of NonAddressable types are not allowed
//CHECK: ([[# @LINE+1]]): error 30027
StructWithParameterBlockArray myArray[3];  // Array of struct containing ParameterBlock array

[numthreads(1, 1, 1)]
void computeMain()
{
    // Empty - we're just testing the declaration
}