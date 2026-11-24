//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -stage fragment -entry main -O0 -validate-ir
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -stage fragment -entry main -validate-ir

// Check that a selection between elements of one storage-buffer array becomes a single access
// through a selected index, like a texture-array selection, so that the module needs no variable
// pointers. The selected index can differ between invocations, so it is marked nonuniform.

// CHECK-NOT: VariablePointers
// CHECK: OpCapability StorageBufferArrayNonUniformIndexing
// CHECK-NOT: VariablePointers
// CHECK: OpDecorate {{.*}} NonUniform
// CHECK-NOT: VariablePointers
// CHECK: OpFunctionEnd

cbuffer Ubo { uint i; uint j; uint condition; };
StructuredBuffer<float> buffers[8];

float main() : SV_Target
{
    StructuredBuffer<float> selected = condition == 0 ? buffers[i] : buffers[j];
    return selected[0];
}
