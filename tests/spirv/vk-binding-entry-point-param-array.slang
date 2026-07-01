//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main -stage compute

// An array-of-resources entry-point parameter forwards its element `count` to the
// explicit-binding reservation, the same way the global parameter path does. Here
// `tex[3]` is bound at (binding 2, set 0) and a sibling buffer is bound at
// (binding 5, set 0); the array's binding is honored as requested.

[numthreads(1, 1, 1)]
[shader("compute")]
void main(
    [[vk::binding(2, 0)]]
    Texture2D tex[3],
    [[vk::binding(5, 0)]]
    RWStructuredBuffer<float> after)
{
    after[0] = tex[0].Load(int3(0, 0, 0)).x;
}

// CHECK-DAG: OpDecorate %entryPointParams_tex Binding 2
// CHECK-DAG: OpDecorate %entryPointParams_tex DescriptorSet 0
// CHECK-DAG: OpDecorate %entryPointParams_after Binding 5
// CHECK-DAG: OpDecorate %entryPointParams_after DescriptorSet 0
