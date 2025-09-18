// Test to check if we emit a warning when [vk::] attributes are used on entry
// point parameters.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -entry computeMain -target cuda
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -entry computeMain -target metal
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -entry computeMain -target cpp
[shader("compute")]
[numthreads(1,1,1)]
void computeMain(
// CHECK: ([[#@LINE+1]]): warning 38010: {{.*}}[[vk::binding]]
    [[vk::binding(5, 4)]] RWStructuredBuffer<uint> test1,
// CHECK: ([[#@LINE+1]]): warning 38010: {{.*}}[[vk::push_constant]]
    [[vk::push_constant]] RWStructuredBuffer<float> test2,
    RWBuffer<float> output
)
{
    output[0] = test2[0];
}
