// builtin-volatile-raytmax-vk-mem-model.slang
//
// Regression test for shader-slang/slang#10528.
// Companion to `builtin-volatile-vk-mem-model.slang`, but exercising
// the `RayTmaxKHR` branch of `needVolatileDecorationForBuiltinVar`
// instead of the subgroup branch. Under the Vulkan memory model the
// `RayTmaxKHR` variable in an intersection shader must NOT carry the
// variable-level `Volatile` decoration, and every load of it must
// instead carry the per-access `Volatile` memory access mask.
//
// Two `RayTCurrent()` calls inside the same intersection entry point
// produce two `OpLoad` instructions, both of which must include the
// mask.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry intersectionMain -stage intersection -emit-spirv-directly -capability vk_mem_model

RWStructuredBuffer<float> output;

// CHECK: OpDecorate %{{[A-Za-z0-9_]+}} BuiltIn RayTmaxKHR
// The `Volatile` token must appear only as a per-access memory operand
// on each `OpLoad`, not as a variable-level decoration.
// CHECK-NOT: OpDecorate %{{[A-Za-z0-9_]+}} Volatile
// CHECK-COUNT-2: OpLoad {{.*}} %{{[A-Za-z0-9_]+}} {{.*}}Volatile

[shader("intersection")]
void intersectionMain()
{
    output[0] = RayTCurrent();
    output[1] = RayTCurrent() + 1.0;
}
