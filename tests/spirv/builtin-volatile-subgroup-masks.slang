// builtin-volatile-subgroup-masks.slang
//
// Regression test for shader-slang/slang#10528. Pins each of the five
// `Subgroup*Mask` enumerators in `needVolatileDecorationForBuiltinVar`
// individually so that a typo swap (e.g. `Ge` <-> `Gt`) or a silent
// removal of any single case in the switch would fail. Same shader,
// two memory models:
//
// - GLSL450: each `Subgroup*Mask` variable carries `OpDecorate ... Volatile`.
// - Vulkan memory model: variables carry NO `Volatile` decoration; instead
//   each `OpLoad` of those builtins carries the `Volatile` memory-access
//   mask. (`SubgroupSize` is included to extend the vk_mem_model coverage
//   beyond `SubgroupLocalInvocationId`, which is exercised by
//   `builtin-volatile-vk-mem-model.slang`.)

//TEST:SIMPLE(filecheck=CHECK_GLSL450): -target spirv-asm -entry rayGenMain -stage raygeneration -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK_VK_MEM_MODEL): -target spirv-asm -entry rayGenMain -stage raygeneration -emit-spirv-directly -capability vk_mem_model

RWStructuredBuffer<uint> output_size;
RWStructuredBuffer<uint4> output_eq;
RWStructuredBuffer<uint4> output_ge;
RWStructuredBuffer<uint4> output_gt;
RWStructuredBuffer<uint4> output_le;
RWStructuredBuffer<uint4> output_lt;

// GLSL450: variable-level Volatile on each mask + SubgroupSize.
// CHECK_GLSL450-DAG: OpDecorate %[[SZ:[A-Za-z0-9_]+]] BuiltIn SubgroupSize
// CHECK_GLSL450-DAG: OpDecorate %[[SZ]] Volatile
// CHECK_GLSL450-DAG: OpDecorate %[[EQ:[A-Za-z0-9_]+]] BuiltIn SubgroupEqMask
// CHECK_GLSL450-DAG: OpDecorate %[[EQ]] Volatile
// CHECK_GLSL450-DAG: OpDecorate %[[GE:[A-Za-z0-9_]+]] BuiltIn SubgroupGeMask
// CHECK_GLSL450-DAG: OpDecorate %[[GE]] Volatile
// CHECK_GLSL450-DAG: OpDecorate %[[GT:[A-Za-z0-9_]+]] BuiltIn SubgroupGtMask
// CHECK_GLSL450-DAG: OpDecorate %[[GT]] Volatile
// CHECK_GLSL450-DAG: OpDecorate %[[LE:[A-Za-z0-9_]+]] BuiltIn SubgroupLeMask
// CHECK_GLSL450-DAG: OpDecorate %[[LE]] Volatile
// CHECK_GLSL450-DAG: OpDecorate %[[LT:[A-Za-z0-9_]+]] BuiltIn SubgroupLtMask
// CHECK_GLSL450-DAG: OpDecorate %[[LT]] Volatile

// Vulkan memory model: BuiltIn decorations only, Volatile mask on each load.
// CHECK_VK_MEM_MODEL-DAG: OpDecorate %{{[A-Za-z0-9_]+}} BuiltIn SubgroupSize
// CHECK_VK_MEM_MODEL-DAG: OpDecorate %{{[A-Za-z0-9_]+}} BuiltIn SubgroupEqMask
// CHECK_VK_MEM_MODEL-DAG: OpDecorate %{{[A-Za-z0-9_]+}} BuiltIn SubgroupGeMask
// CHECK_VK_MEM_MODEL-DAG: OpDecorate %{{[A-Za-z0-9_]+}} BuiltIn SubgroupGtMask
// CHECK_VK_MEM_MODEL-DAG: OpDecorate %{{[A-Za-z0-9_]+}} BuiltIn SubgroupLeMask
// CHECK_VK_MEM_MODEL-DAG: OpDecorate %{{[A-Za-z0-9_]+}} BuiltIn SubgroupLtMask
// CHECK_VK_MEM_MODEL-DAG: OpLoad {{.*}} %{{[A-Za-z0-9_]*}}SubgroupSize{{[A-Za-z0-9_]*}} {{.*}}Volatile
// CHECK_VK_MEM_MODEL-DAG: OpLoad {{.*}} %{{[A-Za-z0-9_]*}}EqMask{{[A-Za-z0-9_]*}} {{.*}}Volatile
// CHECK_VK_MEM_MODEL-DAG: OpLoad {{.*}} %{{[A-Za-z0-9_]*}}GeMask{{[A-Za-z0-9_]*}} {{.*}}Volatile
// CHECK_VK_MEM_MODEL-DAG: OpLoad {{.*}} %{{[A-Za-z0-9_]*}}GtMask{{[A-Za-z0-9_]*}} {{.*}}Volatile
// CHECK_VK_MEM_MODEL-DAG: OpLoad {{.*}} %{{[A-Za-z0-9_]*}}LeMask{{[A-Za-z0-9_]*}} {{.*}}Volatile
// CHECK_VK_MEM_MODEL-DAG: OpLoad {{.*}} %{{[A-Za-z0-9_]*}}LtMask{{[A-Za-z0-9_]*}} {{.*}}Volatile

[shader("raygeneration")]
void rayGenMain()
{
    output_size[0] = WaveGetLaneCount();
    output_eq[0] = WaveGetLaneEqMask();
    output_ge[0] = WaveGetLaneGeMask();
    output_gt[0] = WaveGetLaneGtMask();
    output_le[0] = WaveGetLaneLeMask();
    output_lt[0] = WaveGetLaneLtMask();
}
