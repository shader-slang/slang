// Chained extern constants and polynomial expressions in `vk::binding` arguments.
//
// `DerivedSlot` is an exported extern whose initializer references another extern
// (`BaseSlot`), so resolving it takes more than one link-time resolution pass. The
// standalone program uses the defaults (BaseSlot = 2 → DerivedSlot = 5); linking the
// override module (BaseSlot = 10) re-selects every binding.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute
//TEST:COMPILE: tests/bindings/link-time-vk-binding-chain.slang -o tests/bindings/link-time-vk-binding-chain.slang-module
//TEST:COMPILE: tests/bindings/link-time-vk-binding-chain-lib.slang -o tests/bindings/link-time-vk-binding-chain-lib.slang-module
//TEST:SIMPLE(filecheck=OVERRIDE): -r tests/bindings/link-time-vk-binding-chain.slang-module -r tests/bindings/link-time-vk-binding-chain-lib.slang-module -target spirv-asm -o out.spvasm -stage compute -entry main

export extern static const int BaseSlot = 2;
export extern static const int DerivedSlot = BaseSlot + 3;

[[vk::binding(DerivedSlot, DerivedSlot - 2)]]
RWStructuredBuffer<uint> chainResult;

[[vk::binding(BaseSlot * BaseSlot, BaseSlot * 4)]]
RWStructuredBuffer<uint> powerResult;

[[vk::binding(BaseSlot * DerivedSlot, BaseSlot + DerivedSlot)]]
RWStructuredBuffer<uint> multiResult;

// A chained extern consumed through a non-polynomial builtin operator: resolution
// must survive partial progress (one pass per chain link) and still fold to a constant.
[[vk::binding(DerivedSlot / 2, DerivedSlot == 5 ? 1 : 2)]]
RWStructuredBuffer<uint> divResult;

// CHECK: OpName %[[CHAIN:[A-Za-z_][A-Za-z_0-9]*]] "chainResult"
// CHECK: OpName %[[POWER:[A-Za-z_][A-Za-z_0-9]*]] "powerResult"
// CHECK: OpName %[[MULTI:[A-Za-z_][A-Za-z_0-9]*]] "multiResult"
// CHECK: OpName %[[DIV:[A-Za-z_][A-Za-z_0-9]*]] "divResult"
// CHECK: OpDecorate %[[CHAIN]] Binding 5
// CHECK: OpDecorate %[[CHAIN]] DescriptorSet 3
// CHECK: OpDecorate %[[POWER]] Binding 4
// CHECK: OpDecorate %[[POWER]] DescriptorSet 8
// CHECK: OpDecorate %[[MULTI]] Binding 10
// CHECK: OpDecorate %[[MULTI]] DescriptorSet 7
// CHECK: OpDecorate %[[DIV]] Binding 2
// CHECK: OpDecorate %[[DIV]] DescriptorSet 1
// OVERRIDE: OpName %[[CHAIN2:[A-Za-z_][A-Za-z_0-9]*]] "chainResult"
// OVERRIDE: OpName %[[POWER2:[A-Za-z_][A-Za-z_0-9]*]] "powerResult"
// OVERRIDE: OpName %[[MULTI2:[A-Za-z_][A-Za-z_0-9]*]] "multiResult"
// OVERRIDE: OpName %[[DIV2:[A-Za-z_][A-Za-z_0-9]*]] "divResult"
// OVERRIDE: OpDecorate %[[CHAIN2]] Binding 13
// OVERRIDE: OpDecorate %[[CHAIN2]] DescriptorSet 11
// OVERRIDE: OpDecorate %[[POWER2]] Binding 100
// OVERRIDE: OpDecorate %[[POWER2]] DescriptorSet 40
// OVERRIDE: OpDecorate %[[MULTI2]] Binding 130
// OVERRIDE: OpDecorate %[[MULTI2]] DescriptorSet 23
// OVERRIDE: OpDecorate %[[DIV2]] Binding 6
// OVERRIDE: OpDecorate %[[DIV2]] DescriptorSet 2

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 threadID : SV_DispatchThreadID)
{
    chainResult[threadID.x] = threadID.x;
    powerResult[threadID.x] = threadID.x;
    multiResult[threadID.x] = threadID.x;
    divResult[threadID.x] = threadID.x;
}
