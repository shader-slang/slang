// Test that vk::binding attributes on entry point parameters trigger a warning on spirv target
// This addresses the issue where vk::binding is ignored on entry point parameters

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -entry main -target spirv

[numthreads(32, 1, 1)]
[shader("compute")]
void main(
// CHECK: ([[#@LINE+1]]): warning 38010: {{.*}}vk::binding{{.*}}a
    [[vk::binding(/* binding */ 0, /* set */ 1)]]
    StructuredBuffer<float> a,
// CHECK: ([[#@LINE+1]]): warning 38010: {{.*}}vk::binding{{.*}}b
    [[vk::binding(/* binding */ 3, /* set */ 2)]]
    RWStructuredBuffer<float> b,
)
{
    b[0] = a[0];
}
