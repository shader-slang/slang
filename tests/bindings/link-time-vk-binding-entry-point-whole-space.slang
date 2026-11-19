// An entry-point whole-space parameter (a `ParameterBlock`) consumes whole descriptor
// sets, so its `vk::binding` must resolve to zero — at the entry-point site, against the
// linked program. The standalone program uses the default (SpaceSlot = 0) and places the
// block's contents; a linked override making the resolved binding non-zero must fire
// WholeSpaceParameterRequiresZeroBinding from the entry-point branch. Non-exhaustive
// because the linked run also reports a location-less duplicate and a follow-on
// self-overlap warning from the module-linked declaration.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute
//TEST:COMPILE: tests/bindings/link-time-vk-binding-entry-point-whole-space.slang -o tests/bindings/link-time-vk-binding-entry-point-whole-space.slang-module
//TEST:COMPILE: tests/bindings/link-time-vk-binding-entry-point-whole-space-lib.slang -o tests/bindings/link-time-vk-binding-entry-point-whole-space-lib.slang-module
//DIAGNOSTIC_TEST:SIMPLE(diag=DIAG, non-exhaustive): -r tests/bindings/link-time-vk-binding-entry-point-whole-space.slang-module -r tests/bindings/link-time-vk-binding-entry-point-whole-space-lib.slang-module -target spirv-asm -stage compute -entry main

export extern static const int SpaceSlot = 0;

struct Params
{
    RWStructuredBuffer<uint> data;
};

[shader("compute")]
[numthreads(1, 1, 1)]
void main(
    uint3 threadID : SV_DispatchThreadID,
    [[vk::binding(SpaceSlot, 3)]] ParameterBlock<Params> block)
/*DIAG:
          ^^^^^^^ whole descriptor set requires binding 0
          ^^^^^^^ shader parameter 'block' consumes whole descriptor sets, so the binding must be in the form '[[vk::binding(0, ...)]]'; the non-zero binding '3' is not allowed
*/
{
    block.data[threadID.x] = threadID.x;
}

// CHECK: OpName %[[DATA:[A-Za-z_][A-Za-z_0-9]*]] "entryPointParams.block.data"
// CHECK: OpDecorate %[[DATA]] Binding 0
// CHECK: OpDecorate %[[DATA]] DescriptorSet 3
