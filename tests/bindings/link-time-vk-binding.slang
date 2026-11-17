// Link-time constants in `vk::binding` arguments.
//
// The default program picks binding 7 / set 1; linking a module that
// exports LayoutVariant = 4 selects binding 9 / set 3.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute
//TEST:COMPILE: tests/bindings/link-time-vk-binding.slang -o tests/bindings/link-time-vk-binding.slang-module
//TEST:COMPILE: tests/bindings/link-time-vk-binding-lib.slang -o tests/bindings/link-time-vk-binding-lib.slang-module
//TEST:SIMPLE(filecheck=OVERRIDE): -r tests/bindings/link-time-vk-binding.slang-module -r tests/bindings/link-time-vk-binding-lib.slang-module -target spirv-asm -o out.spvasm -stage compute -entry main

export extern static const int LayoutVariant = 3;
static const int OutputSlot = LayoutVariant == 3 ? 7 : 9;

[[vk::binding(OutputSlot, OutputSlot - 6)]]
RWStructuredBuffer<uint> result;

// CHECK: OpName %[[RESULT:[A-Za-z_][A-Za-z_0-9]*]] "result"
// CHECK: OpDecorate %[[RESULT]] Binding 7
// CHECK: OpDecorate %[[RESULT]] DescriptorSet 1
// WGSL: @binding(7)
// WGSL: @group(1)
// OVERRIDE: OpName %[[RESULT2:[A-Za-z_][A-Za-z_0-9]*]] "result"
// OVERRIDE: OpDecorate %[[RESULT2]] Binding 9
// OVERRIDE: OpDecorate %[[RESULT2]] DescriptorSet 3

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 threadID : SV_DispatchThreadID)
{
    result[threadID.x] = threadID.x;
}
