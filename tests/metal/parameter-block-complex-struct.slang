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

// CHECK-ASM: define void @computeMain

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // CHECK: {{.*}}->buffer{{.*}}
    outputBuffer[0] = simpleBlock.buffer[0];
    
    // CHECK: {{.*}}[int(1)])->buffer{{.*}}
    ResourceStruct fromArray = arrayBlock[1];
    outputBuffer[1] = fromArray.buffer[1];
        
    // CHECK: {{.*}}->nested{{.*}}[int(1)])->resources{{.*}}[int(1)])->buffer{{.*}}
    outputBuffer[3] = complexBlock.nested[1].resources[1].buffer[3];
    
    // CHECK: {{.*}}[int(0)])->nested{{.*}}[int(2)])->resources{{.*}}[int(0)])->buffer{{.*}}
    ComplexStruct complexFromArray = arrayComplexBlock[0];
    outputBuffer[4] = complexFromArray.nested[2].resources[0].buffer[4];
    
    // CHECK: {{.*}}->directBuffer{{.*}}
    outputBuffer[5] = complexBlock.directBuffer[5];
    


    // // TODO: This crashes the compiler
    // NestedStruct nested = nestedBlock;
    // outputBuffer[2] = nested.resources[0].buffer[2];
}