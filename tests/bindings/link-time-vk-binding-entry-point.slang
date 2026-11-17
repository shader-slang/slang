// Link-time constants in `vk::binding` on entry-point parameters.
//
// The default program binds the entry-point parameter at binding 2; linking the
// override module (Slot = 9) re-selects binding 9 after linking.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute
//TEST:COMPILE: tests/bindings/link-time-vk-binding-entry-point.slang -o tests/bindings/link-time-vk-binding-entry-point.slang-module
//TEST:COMPILE: tests/bindings/link-time-vk-binding-entry-point-lib.slang -o tests/bindings/link-time-vk-binding-entry-point-lib.slang-module
//TEST:SIMPLE(filecheck=OVERRIDE): -r tests/bindings/link-time-vk-binding-entry-point.slang-module -r tests/bindings/link-time-vk-binding-entry-point-lib.slang-module -target spirv-asm -o out.spvasm -stage compute -entry main

export extern static const int Slot = 2;

// CHECK: OpName %[[RESULT:[A-Za-z_][A-Za-z_0-9]*]] "entryPointParams.result"
// CHECK: OpDecorate %[[RESULT]] Binding 2
// CHECK: OpDecorate %[[RESULT]] DescriptorSet 1
// OVERRIDE: OpName %[[RESULT2:[A-Za-z_][A-Za-z_0-9]*]] "entryPointParams.result"
// OVERRIDE: OpDecorate %[[RESULT2]] Binding 9
// OVERRIDE: OpDecorate %[[RESULT2]] DescriptorSet 1

[shader("compute")]
[numthreads(1, 1, 1)]
void main(
    uint3 threadID : SV_DispatchThreadID,
    [[vk::binding(Slot, 1)]] RWStructuredBuffer<uint> result)
{
    result[threadID.x] = threadID.x;
}
