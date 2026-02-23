// Test that StructuredBuffer of ParameterBlock (NonAddressable type) is not allowed

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -allow-glsl -stage compute -entry computeMain

// Direct ParameterBlock in StructuredBuffer - should error
StructuredBuffer<ParameterBlock<Texture2D>> myBuffer1;
//CHECK:                                    ^^^^^^^^^ non-addressable type cannot be used in StructuredBuffer
//CHECK:                                    ^^^^^^^^^ 'ParameterBlock<Texture2D<vector<float,4>>>' is non-addressable and cannot be used in StructuredBuffer

// Struct containing ParameterBlock in StructuredBuffer - should error  
struct StructWithParameterBlock
{
    ParameterBlock<Texture2D> block;
    int value;
}

StructuredBuffer<StructWithParameterBlock> myBuffer2;
//CHECK:                                   ^^^^^^^^^ non-addressable type cannot be used in StructuredBuffer
//CHECK:                                   ^^^^^^^^^ 'StructWithParameterBlock' is non-addressable and cannot be used in StructuredBuffer

// Nested struct containing ParameterBlock - should error
struct NestedStruct
{
    StructWithParameterBlock nested;
    float data;
}

StructuredBuffer<NestedStruct> myBuffer3;
//CHECK:                       ^^^^^^^^^ non-addressable type cannot be used in StructuredBuffer
//CHECK:                       ^^^^^^^^^ 'NestedStruct' is non-addressable and cannot be used in StructuredBuffer

// Struct containing array of ParameterBlock - should error
struct StructWithParameterBlockArray
{
    ParameterBlock<Texture2D> blocks[2];
//CHECK:                      ^^^^^^ arrays of non-addressable type not allowed
//CHECK:                      ^^^^^^ Arrays of non-addressable type 'ParameterBlock<Texture2D<vector<float,4>>>' are not allowed
//CHECK:                      ^^^^^^ arrays of non-addressable type not allowed
//CHECK:                      ^^^^^^ Arrays of non-addressable type 'ParameterBlock<Texture2D<vector<float,4>>>' are not allowed
    int value;
}

StructuredBuffer<StructWithParameterBlockArray> myBuffer4;
//CHECK:                                        ^^^^^^^^^ non-addressable type cannot be used in StructuredBuffer
//CHECK:                                        ^^^^^^^^^ 'StructWithParameterBlockArray' is non-addressable and cannot be used in StructuredBuffer

[numthreads(1, 1, 1)]
void computeMain()
{
    // Empty - we're just testing the declarations
}