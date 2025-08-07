//TEST:SIMPLE(filecheck=CHECK):  -stage compute -entry computeMain -target metal
//TEST:SIMPLE(filecheck=CHECK-ASM): -target metallib

// Test parameter blocks with complex nested struct and array types
// This test verifies that resource types within structs and arrays
// are preserved in Metal.

struct ResourceStruct {
    RWStructuredBuffer<int> buffer;
    Texture2D<float4> texture;
}

struct NestedStruct {
    Array<ResourceStruct, 2> resources;
    int padding;
}

struct ComplexStruct {
    Array<NestedStruct, 3> nested;
    RWStructuredBuffer<float> directBuffer;
}

// Test various parameter block configurations
ParameterBlock<ResourceStruct> simpleBlock;
ParameterBlock<Array<ResourceStruct, 4>> arrayBlock; 
ParameterBlock<NestedStruct> nestedBlock;
ParameterBlock<ComplexStruct> complexBlock;
ParameterBlock<Array<ComplexStruct, 2>> arrayComplexBlock;

RWStructuredBuffer<float> outputBuffer;

// CHECK-ASM: define void @main_kernel

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // CHECK: simpleBlock_0->buffer_0
    ResourceStruct simple = simpleBlock;
    outputBuffer[0] = simple.buffer[0];
    
    // CHECK: arrayBlock_0)[int(1)])->buffer_0
    ResourceStruct fromArray = arrayBlock[1];
    outputBuffer[1] = fromArray.buffer[1];
    
    // CHECK: nestedBlock_0->resources_0[int(0)])->buffer_0
    NestedStruct nested = nestedBlock;
    outputBuffer[2] = nested.resources[0].buffer[2];
    
    // CHECK: complexBlock_0->nested_0[int(1)])->resources_0[int(1)])->buffer_0
    ComplexStruct complex = complexBlock;
    outputBuffer[3] = complex.nested[1].resources[1].buffer[3];
    
    // CHECK: arrayComplexBlock_0)[int(0)])->nested_0[int(2)])->resources_0[int(0)])->buffer_0
    ComplexStruct complexFromArray = arrayComplexBlock[0];
    outputBuffer[4] = complexFromArray.nested[2].resources[0].buffer[4];
    
    // CHECK: complexBlock_0->directBuffer_0
    outputBuffer[5] = complex.directBuffer[5];
    
}