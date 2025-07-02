// vk-binding-with-register-no-warning.slang

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target metallib -matrix-layout-row-major -entry main

// Test that no spurious warnings are generated when vk::binding is present alongside register
// This addresses issue #7553 where the compiler incorrectly warned about missing vk::binding

// Should NOT generate warning - has both vk::binding and register
// CHECK-NOT: ([[# @LINE+1]]): warning 39029
[[vk::binding(0, 0)]] cbuffer ConstantBuffer : register(b0)
{
    float4 testValue;
};

// Should NOT generate warning - has both vk::binding and register  
// CHECK-NOT: ([[# @LINE+1]]): warning 39029
[[vk::binding(1, 0)]] Texture2D tex : register(t1);

// Should NOT generate warning - has both vk::binding and register
// CHECK-NOT: ([[# @LINE+1]]): warning 39029
[[vk::binding(2, 0)]] SamplerState sampler : register(s2);

// Should generate warning - has register but no vk::binding
// CHECK: ([[# @LINE+1]]): warning 39029
Texture2D texNoBinding : register(t3);

// Should generate warning - has register but no vk::binding
// CHECK: ([[# @LINE+1]]): warning 39029
ConstantBuffer<float4> cbNoBinding : register(b4);

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
}