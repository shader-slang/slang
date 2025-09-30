//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute
// Test that unsized types in StructuredBuffer<T> cause compilation errors

struct UnsizedStruct 
{ 
    float2 ua[]; 
};

// CHECK: ([[# @LINE+1]]): error {{[0-9]+}}
RWStructuredBuffer<UnsizedStruct> buffer1;

// CHECK: ([[# @LINE+1]]): error {{[0-9]+}}
StructuredBuffer<UnsizedStruct> buffer2;

[shader("compute")][numthreads(1, 1, 1)]
void main(uint3 threadId : SV_DispatchThreadID)
{
    // This should fail at compile time, not at SPIRV validation
    buffer1[0].ua[0] = 1.0;
    float x = buffer2[0].ua[0].x;
}