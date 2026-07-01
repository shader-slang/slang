// Regression test for https://github.com/shader-slang/slang/issues/11861
//
// `[[vk::binding(...)]]` on an entry-point parameter whose type is a struct that
// *contains* resources must be honored (the binder places the fields) and must NOT
// be diagnosed as ignored (E38010). `-warnings-as-errors 38010` makes a spurious
// warning fatal, so this test doubles as the "no E38010" regression guard: if the
// diagnostic fires, compilation fails and no SPIR-V is produced.
//
// Covers the three recursion paths added to `isVkBindingCompatibleEntryPointParameterType`:
// a flat struct, a nested struct, and a struct that inherits its resource from a base.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute -warnings-as-errors 38010

// Flat struct-of-resources (the reported case).
struct Resources
{
    Texture2D<float4> tex;
    SamplerState samp;
};

// Nested struct: the descriptor is reached through a field of struct type.
struct Inner
{
    Texture2D<float4> tex;
};
struct Outer
{
    Inner inner;
    SamplerState samp;
};

// Inherited: the descriptor comes from a base struct.
struct Base
{
    Texture2D<float4> tex;
};
struct Derived : Base
{
    SamplerState samp;
};

RWStructuredBuffer<float4> outputBuffer;

[numthreads(1, 1, 1)]
[shader("compute")]
void main(
    [[vk::binding(/* binding */ 2, /* set */ 1)]] uniform Resources resources,
    [[vk::binding(/* binding */ 0, /* set */ 3)]] uniform Outer outer,
    [[vk::binding(/* binding */ 0, /* set */ 5)]] uniform Derived derived)
{
    float2 uv = float2(0.0, 0.0);
    outputBuffer[0] = resources.tex.SampleLevel(resources.samp, uv, 0.0) +
                      outer.inner.tex.SampleLevel(outer.samp, uv, 0.0) +
                      derived.tex.SampleLevel(derived.samp, uv, 0.0);
}

// Flat struct: tex at (binding 2, set 1), samp auto-follows at (binding 3, set 1).
// CHECK-DAG: OpDecorate %entryPointParams_resources_tex Binding 2
// CHECK-DAG: OpDecorate %entryPointParams_resources_tex DescriptorSet 1
// CHECK-DAG: OpDecorate %entryPointParams_resources_samp Binding 3
// CHECK-DAG: OpDecorate %entryPointParams_resources_samp DescriptorSet 1

// Nested struct: inner.tex at (binding 0, set 3), samp auto-follows at (binding 1, set 3).
// CHECK-DAG: OpDecorate %entryPointParams_outer_inner_tex Binding 0
// CHECK-DAG: OpDecorate %entryPointParams_outer_inner_tex DescriptorSet 3
// CHECK-DAG: OpDecorate %entryPointParams_outer_samp Binding 1
// CHECK-DAG: OpDecorate %entryPointParams_outer_samp DescriptorSet 3

// Inherited struct: base tex at (binding 0, set 5), samp auto-follows at (binding 1, set 5).
// CHECK-DAG: OpDecorate %entryPointParams_derived_base_tex Binding 0
// CHECK-DAG: OpDecorate %entryPointParams_derived_base_tex DescriptorSet 5
// CHECK-DAG: OpDecorate %entryPointParams_derived_samp Binding 1
// CHECK-DAG: OpDecorate %entryPointParams_derived_samp DescriptorSet 5
