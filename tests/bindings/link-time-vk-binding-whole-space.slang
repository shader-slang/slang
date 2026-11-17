// A whole-space parameter (a `ParameterBlock`) consumes whole descriptor sets, so its
// `vk::binding` must be 0. That constraint must still fire when the binding is a link-time
// constant that only resolves to non-zero after a module defining it is linked: the
// standalone program uses the default 0 and compiles cleanly; linking SpaceSlot = 3 makes
// the resolved binding non-zero and must diagnose it. Non-exhaustive because the layout
// cascade also reports the same whole-space error from a location-less site and a
// follow-on overlap warning between the parameter and itself.

//TEST:COMPILE: tests/bindings/link-time-vk-binding-whole-space.slang -o tests/bindings/link-time-vk-binding-whole-space.slang-module
//TEST:COMPILE: tests/bindings/link-time-vk-binding-whole-space-lib.slang -o tests/bindings/link-time-vk-binding-whole-space-lib.slang-module
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK, non-exhaustive): -r tests/bindings/link-time-vk-binding-whole-space.slang-module -r tests/bindings/link-time-vk-binding-whole-space-lib.slang-module -target spirv-asm -stage compute -entry main

export extern static const int SpaceSlot = 0;

struct Params
{
    RWStructuredBuffer<uint> data;
};

[[vk::binding(SpaceSlot, 2)]]
/*CHECK:
      ^^^^^^^ whole descriptor set requires binding 0
      ^^^^^^^ shader parameter 'params' consumes whole descriptor sets, so the binding must be in the form '[[vk::binding(0, ...)]]'; the non-zero binding '3' is not allowed
*/
ParameterBlock<Params> params;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 threadID : SV_DispatchThreadID)
{
    params.data[threadID.x] = threadID.x;
}
