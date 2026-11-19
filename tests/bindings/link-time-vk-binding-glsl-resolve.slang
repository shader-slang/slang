// The GLSL-source positive path: a symbolic `layout(binding = ...)` qualifier stored at
// check time and resolved at layout into concrete decorations, plus a `layout(set = N)`
// without any `binding` qualifier — the null-binding-means-index-0 contract. The linked
// override re-selects both values through the same qualifier-merge path.

//TEST:SIMPLE_EX(filecheck=CHECK): -lang glsl tests/bindings/link-time-vk-binding-glsl-resolve.slang -stage fragment -entry fragmentMain -target spirv-asm
//TEST:COMPILE: -lang glsl tests/bindings/link-time-vk-binding-glsl-resolve.slang -o tests/bindings/link-time-vk-binding-glsl-resolve.slang-module
//TEST:COMPILE: tests/bindings/link-time-vk-binding-glsl-resolve-lib.slang -o tests/bindings/link-time-vk-binding-glsl-resolve-lib.slang-module
//TEST:SIMPLE_EX(filecheck=OVERRIDE): -r tests/bindings/link-time-vk-binding-glsl-resolve.slang-module -r tests/bindings/link-time-vk-binding-glsl-resolve-lib.slang-module -lang glsl tests/bindings/link-time-vk-binding-glsl-resolve.slang -stage fragment -entry fragmentMain -target spirv-asm

#version 450

extern static const int SLOT = 3;

layout(binding = SLOT, set = SLOT - 2) buffer Output
{
    uint result[];
};

layout(set = 5) buffer Data
{
    uint data[];
};

// CHECK: OpName %[[OUT:[A-Za-z_][A-Za-z_0-9]*]] "Output"
// CHECK: OpName %[[DATA:[A-Za-z_][A-Za-z_0-9]*]] "Data"
// CHECK: OpDecorate %[[OUT]] Binding 3
// CHECK: OpDecorate %[[OUT]] DescriptorSet 1
// CHECK: OpDecorate %[[DATA]] Binding 0
// CHECK: OpDecorate %[[DATA]] DescriptorSet 5
// OVERRIDE: OpName %[[OUT2:[A-Za-z_][A-Za-z_0-9]*]] "Output"
// OVERRIDE: OpName %[[DATA2:[A-Za-z_][A-Za-z_0-9]*]] "Data"
// OVERRIDE: OpDecorate %[[OUT2]] Binding 8
// OVERRIDE: OpDecorate %[[OUT2]] DescriptorSet 6
// OVERRIDE: OpDecorate %[[DATA2]] Binding 0
// OVERRIDE: OpDecorate %[[DATA2]] DescriptorSet 5

void fragmentMain()
{
    result[0] = data[0];
}
