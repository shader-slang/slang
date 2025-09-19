// Test to check if we emit a warning when unhandled modifiers are applied to
// entry point parameters.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -entry computeMain -target cuda
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -entry computeMain -target metal
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -entry computeMain -target cpp
[shader("compute")]
[numthreads(1,1,1)]
void computeMain(
// CHECK: ([[#@LINE+1]]): warning 38010: {{.*}}vk::binding
    [[vk::binding(5, 4)]] RWStructuredBuffer<uint> test1,
// CHECK: ([[#@LINE+1]]): warning 38010: {{.*}}vk::push_constant
    [[vk::push_constant]] RWStructuredBuffer<float> test2,
// CHECK: ([[#@LINE+1]]): warning 38010: {{.*}}register
    RWStructuredBuffer<float> test3 : register(r0),
// CHECK: ([[#@LINE+1]]): warning 38010: {{.*}}packoffset
    float test4 : packoffset(c4),
    RWBuffer<float> output
)
{
    output[0] = test2[0];
}
