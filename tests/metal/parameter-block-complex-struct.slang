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

struct ParentStruct {
    struct ChildStruct {
        Texture2D<float> texture;
    } c;
}

struct OtherParent {
      ParentStruct::ChildStruct c2;
}

// Test various parameter block configurations
ParameterBlock<ResourceStruct> simpleBlock;
ParameterBlock<Array<ResourceStruct, 4>> arrayBlock; 
ParameterBlock<NestedStruct> nestedBlock;
ParameterBlock<ComplexStruct> complexBlock;
ParameterBlock<Array<ComplexStruct, 2>> arrayComplexBlock;
ParameterBlock<ParentStruct> parentBlock;

RWStructuredBuffer<float> outputBuffer;

// CHECK-ASM: define void @computeMain

void func(ParentStruct::ChildStruct c) {
    outputBuffer[0] = c.texture.Load(int3(0));
}

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
    
    // Test parameter block accessed via a local variable
    // CHECK: {{.*}}->buffer{{.*}}
    NestedStruct nested = nestedBlock;
    outputBuffer[6] = nested.resources[0].buffer[2];

    // Test function with parameter block containing a struct with a resource type
    // CHECK: {{.*}}func{{.*}}->texture{{.*}}
    func(parentBlock.c);

    // Test function with a copy of the parameter block
    // CHECK: {{.*}}func{{.*}}->texture{{.*}}
    OtherParent p;
    p.c2 = parentBlock.c;
    func(p.c2);
}