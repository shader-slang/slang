// vk-binding-with-register-no-warning.slang

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target metallib -matrix-layout-row-major -entry main

// Test that no spurious warnings are generated when vk::binding is present alongside register
// This addresses issue #7553 where the compiler incorrectly warned about missing vk::binding

// Should NOT generate warning - has both vk::binding and register
// CHECK-NOT: warning{{.*}}39029{{.*}}testValue
[[vk::binding(0, 0)]] cbuffer ConstantBuffer : register(b0)
{
    float4 testValue;
};

// Should NOT generate warning - has both vk::binding and register
// CHECK-NOT: warning{{.*}}39029{{.*}}'tex'
[[vk::binding(1, 0)]] Texture2D tex : register(t1);

// Should NOT generate warning - has both vk::binding and register
// CHECK-NOT: warning{{.*}}39029{{.*}}'sampler'
[[vk::binding(2, 0)]] SamplerState sampler : register(s2);

// Should generate warning - has register but no vk::binding
// CHECK: warning{{.*}}39029
// CHECK: texNoBinding
Texture2D texNoBinding : register(t3);

// Should generate warning - has register but no vk::binding
// CHECK: warning{{.*}}39029
// CHECK: cbNoBinding
ConstantBuffer<float4> cbNoBinding : register(b4);

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
}