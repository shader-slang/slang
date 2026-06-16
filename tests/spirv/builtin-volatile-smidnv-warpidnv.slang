// builtin-volatile-smidnv-warpidnv.slang
//
// Regression test for shader-slang/slang#10528.
// `SMIDNV` and `WarpIDNV` are listed in the Vulkan SPIR-V env spec
// as Volatile-required in raytracing-volatile stages. Slang has no
// built-in intrinsic that lowers to them today (no
// `__intrinsic_asm` / `spirv_asm` referent in `hlsl.meta.slang`),
// so a `case` typo (e.g. swapping `SMIDNV` and `WarpIDNV`) or a
// silent removal would not be caught by any of the
// hlsl.meta.slang-driven tests.
//
// This test reaches the cases via a hand-written `spirv_asm` block
// that emits `OpLoad builtin(SMIDNV:uint)` and
// `OpLoad builtin(WarpIDNV:uint)` directly from a raygen entry
// point. Each variable must end up with the `Volatile` decoration
// under the GLSL450 memory model.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry rayGenMain -stage raygeneration -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK_VK_MM): -target spirv-asm -entry rayGenMain -stage raygeneration -emit-spirv-directly -capability vk_mem_model

RWStructuredBuffer<uint> output;

uint readSmId()
{
    return spirv_asm
    {
        OpCapability ShaderSMBuiltinsNV;
        OpExtension "SPV_NV_shader_sm_builtins";
        result:$$uint = OpLoad builtin(SMIDNV:uint)
    };
}

uint readWarpId()
{
    return spirv_asm
    {
        OpCapability ShaderSMBuiltinsNV;
        OpExtension "SPV_NV_shader_sm_builtins";
        result:$$uint = OpLoad builtin(WarpIDNV:uint)
    };
}

// Both builtin variables get a `Volatile` decoration under GLSL450
// in raygen. Use CHECK-DAG so this is order-independent.
// CHECK-DAG: OpDecorate %[[SM:[A-Za-z0-9_]+]] BuiltIn SMIDNV
// CHECK-DAG: OpDecorate %[[SM]] Volatile
// CHECK-DAG: OpDecorate %[[WARP:[A-Za-z0-9_]+]] BuiltIn WarpIDNV
// CHECK-DAG: OpDecorate %[[WARP]] Volatile

// Under the Vulkan memory model the variable-level `Volatile`
// decoration is invalid; the per-access mask on each `OpLoad`
// carries the requirement instead. Pin both halves: BuiltIn
// decorations present, no variable-level Volatile, and each
// `OpLoad` carries the Volatile mask.
// CHECK_VK_MM-DAG: OpDecorate %[[SM2:[A-Za-z0-9_]+]] BuiltIn SMIDNV
// CHECK_VK_MM-DAG: OpDecorate %[[WARP2:[A-Za-z0-9_]+]] BuiltIn WarpIDNV
// CHECK_VK_MM-NOT: OpDecorate %{{[A-Za-z0-9_]+}} Volatile
// CHECK_VK_MM-DAG: OpLoad {{.*}} %[[SM2]] {{.*}}Volatile
// CHECK_VK_MM-DAG: OpLoad {{.*}} %[[WARP2]] {{.*}}Volatile

[shader("raygeneration")]
void rayGenMain()
{
    output[0] = readSmId();
    output[1] = readWarpId();
}
