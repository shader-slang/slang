// Test that StructuredBuffer of ParameterBlock (NonAddressable type) is not allowed

//TEST:SIMPLE(filecheck=CHECK): -target spirv -allow-glsl -stage compute -entry computeMain

// Direct ParameterBlock in StructuredBuffer - should error
//CHECK: ([[# @LINE+1]]): error 30028
StructuredBuffer<ParameterBlock<Texture2D>> myBuffer1;

// Struct containing ParameterBlock in StructuredBuffer - should error  
struct StructWithParameterBlock
{
    ParameterBlock<Texture2D> block;
    int value;
}

//CHECK: ([[# @LINE+1]]): error 30028
StructuredBuffer<StructWithParameterBlock> myBuffer2;

// Nested struct containing ParameterBlock - should error
struct NestedStruct
{
    StructWithParameterBlock nested;
    float data;
}

//CHECK: ([[# @LINE+1]]): error 30028
StructuredBuffer<NestedStruct> myBuffer3;

// Struct containing array of ParameterBlock - should error
struct StructWithParameterBlockArray
{
    ParameterBlock<Texture2D> blocks[2];
    int value;
}

//CHECK: ([[# @LINE+1]]): error 30028
StructuredBuffer<StructWithParameterBlockArray> myBuffer4;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Empty - we're just testing the declarations
}