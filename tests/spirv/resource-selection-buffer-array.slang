//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -stage fragment -entry main -O0 -validate-ir
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -stage fragment -entry main -validate-ir

// This test checks that storage-buffer array selections keep their variable-pointer representation
// without acquiring a nonuniform descriptor-indexing requirement.

// CHECK-NOT: NonUniform
// CHECK: OpCapability VariablePointersStorageBuffer
// CHECK-NOT: NonUniform
// CHECK: OpMemoryModel
// CHECK-NOT: NonUniform
// CHECK: OpFunctionEnd

cbuffer Ubo { uint i; uint j; uint condition; };
StructuredBuffer<float> buffers[8];

float main() : SV_Target
{
    StructuredBuffer<float> selected = condition == 0 ? buffers[i] : buffers[j];
    return selected[0];
}
