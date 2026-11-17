// A combined GLSL `layout(binding = N, set = M)` parses into one binding modifier and one
// set modifier that merge into a single binding attribute and re-validate in turn; a later
// pass must not clobber the value an earlier pass stored. The binding-first order pins the
// previously untested direction: the set pass must leave `binding = 3` intact, and the
// resolved decorations must show both values surviving the merge. This path needs no
// link-time constants — concrete indices pin the merge/no-clobber behavior the `IntVal`
// field migration depends on; the symbolic-value side is covered by the resolve and
// offset tests.

//TEST:SIMPLE_EX(filecheck=CHECK): -lang glsl tests/bindings/link-time-vk-binding-glsl-merge.slang -stage fragment -entry fragmentMain -target spirv-asm

#version 450

layout(binding = 3, set = 5) buffer Output
{
    uint result[];
};

// CHECK: OpName %[[RESULT:[A-Za-z_][A-Za-z_0-9]*]] "Output"
// CHECK: OpDecorate %[[RESULT]] Binding 3
// CHECK: OpDecorate %[[RESULT]] DescriptorSet 5

void fragmentMain()
{
    result[0] = 1;
}
