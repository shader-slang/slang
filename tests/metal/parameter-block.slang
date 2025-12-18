//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target metal
//TEST:SIMPLE(filecheck=CHECK-ASM): -stage compute -entry computeMain -target metallib

// This test verifies that resource types within structs and arrays
// are preserved in Metal. This is done by lowering resource tyeps into
// descriptor handle types during lower-buffer-element-type pass.

struct Val
{
    RWStructuredBuffer<float> buffer;
}

struct Val_f
{
    float f;
}

struct ResourceStruct {
    RWStructuredBuffer<float> buffer;
    Texture2D<float4> texture;
}

struct NestedStruct {
    Array<ResourceStruct, 2> resources;
    ResourceStruct resource;
    int padding;
}

struct ComplexStruct {
    Array<NestedStruct, 3> nested;
    RWStructuredBuffer<float> directBuffer;
}

struct ParentStruct {
    struct ChildStruct {
        Texture2D<float> texture;
        int i;
        float f;
    } c;

    Array<Texture2D<float>, 2> tex_array;
}

struct OtherParent {
      ParentStruct::ChildStruct c2;
}

struct StructWithParameterBlock {
    RWStructuredBuffer<float> buffer;
    ParameterBlock<Array<Val, 2>> arr;
}

uniform StructWithParameterBlock struct_with_parameter_block;


// Test various parameter block configurations
ParameterBlock<ResourceStruct> simpleBlock;
ParameterBlock<Array<ResourceStruct, 4>> arrayBlock; 
ParameterBlock<NestedStruct> nestedBlock;
ParameterBlock<ComplexStruct> complexBlock;
ParameterBlock<Array<ComplexStruct, 2>> arrayComplexBlock;
ParameterBlock<ParentStruct> parentBlock;

ParameterBlock<RWStructuredBuffer<Val_f>> valBlock;


RWStructuredBuffer<float> outputBuffer;

void func(ParentStruct::ChildStruct c) {
    outputBuffer[0] = c.texture.Load(int3(0));
    Texture2D<float> tex = c.texture;
    outputBuffer[0] = tex.Load(int3(0));

}

void func2(ParentStruct p) {
    Array<Texture2D<float>, 2> tex_array_local = p.tex_array;
    outputBuffer[1] = tex_array_local[0].Load(int3(0));
    outputBuffer[2] = p.c.texture.Load(int3(3));
}

void func3(Array<Texture2D<float>, 2> arr) {
    outputBuffer[3] = arr[0].Load(int3(0));
}

void func4(ParameterBlock<ParentStruct> block) {
    ParentStruct p = block;
    outputBuffer[8] = p.c.texture.Load(int3(0));
}

// CHECK-ASM: {{.*}} @computeMain
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{

    // CHECK: {{.*}}->buffer{{.*}}
    outputBuffer[0] = simpleBlock.buffer[0];
    
    // CHECK: arrayBlock{{.*}}->data{{.*}}[int(1)].buffer
    ResourceStruct fromArray = arrayBlock[1];
    outputBuffer[1] = fromArray.buffer[1];
        
    // CHECK: complexBlock{{.*}}->nested{{.*}}->data{{.*}}[int(1)])->resources{{.*}})->data{{.*}}[int(1)])->buffer{{.*}}
    outputBuffer[3] = complexBlock.nested[1].resources[1].buffer[3];
    
    // CHECK: {{.*}}arrayComplexBlock{{.*}}->data{{.*}}[int(0)])->nested{{.*}})->data{{.*}}[int(2)])->resources{{.*}})->data{{.*}}[int(0)].buffer
    ComplexStruct complexFromArray = arrayComplexBlock[0];
    outputBuffer[4] = complexFromArray.nested[2].resources[0].buffer[4];
    
    // CHECK: {{.*}}->directBuffer{{.*}}
    outputBuffer[5] = complexBlock.directBuffer[5];
    
    // Test parameter block accessed via a local variable
    // CHECK: nestedBlock{{.*}}->resource{{.*}}.buffer_0
    ResourceStruct resStruct = nestedBlock.resource;
    outputBuffer[6] = resStruct.buffer[2];

    // CHECK: {{.*}}valBlock{{.*}}
    outputBuffer[7] = valBlock[0].f;

    // Test parameter block inside a struct
    // CHECK: {{.*}}struct_with_parameter_block_arr{{.*}}
    StructWithParameterBlock local = struct_with_parameter_block;
    outputBuffer[8] = local.arr[0].buffer[0];

    // Test function with parameter block containing a struct with a resource type
    func(parentBlock.c);

    // Test function with a copy of the parameter block
    OtherParent other_p;
    other_p.c2 = parentBlock.c;
    func(other_p.c2);

    ParentStruct p;
    p = parentBlock;
    func2(p);

    // Test function with a field from the parameter block
    func3(p.tex_array);

    // Test function with a parameter block in the function signature
    func4(parentBlock);
}