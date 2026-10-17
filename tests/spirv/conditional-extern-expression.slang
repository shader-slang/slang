//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm
//TEST:REFLECTION(filecheck=REFLECTION):-target spirv -no-codegen

// Check bindings and reflection when extern defaults use arithmetic or another constant.
// Both conditions are true, so both buffers must be present.
static const bool enabled = true;
extern static const int count = 1 + 0;
extern static const bool useAlias = enabled;
uniform Conditional<ConstantBuffer<float>, count == 1> arithmeticBuffer;
uniform Conditional<ConstantBuffer<float>, useAlias> aliasBuffer;

[shader("fragment")]
float main() : SV_Target
{
    return arithmeticBuffer.get().value + aliasBuffer.get().value;
}

// SPIRV-DAG: OpDecorate %arithmeticBuffer Binding 0
// SPIRV-DAG: OpDecorate %arithmeticBuffer DescriptorSet 0
// SPIRV-DAG: OpDecorate %aliasBuffer Binding 1
// SPIRV-DAG: OpDecorate %aliasBuffer DescriptorSet 0
// SPIRV-DAG: OpAccessChain {{.*}} %arithmeticBuffer
// SPIRV-DAG: OpAccessChain {{.*}} %aliasBuffer

// REFLECTION: "name": "arithmeticBuffer",
// REFLECTION-NEXT: "binding": {"kind": "descriptorTableSlot", "index": 0},
// REFLECTION-NEXT: "type": {
// REFLECTION-NEXT: "kind": "constantBuffer",
// REFLECTION: "name": "aliasBuffer",
// REFLECTION-NEXT: "binding": {"kind": "descriptorTableSlot", "index": 1},
// REFLECTION-NEXT: "type": {
// REFLECTION-NEXT: "kind": "constantBuffer",
