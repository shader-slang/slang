// Regression test for https://github.com/shader-slang/slang/issues/11861
//
// `[[vk::binding(...)]]` on an entry-point parameter whose type is a struct that
// *contains* resources must be honored (the binder places the fields) and must NOT
// be diagnosed as ignored (E38010). `-warnings-as-errors 38010` makes a spurious
// warning fatal, so this test doubles as the "no E38010" regression guard: if the
// diagnostic fires, compilation fails and no SPIR-V is produced.
//
// Covers the distinct traversals of `isVkBindingCompatibleEntryPointParameterType`:
// a flat struct, a nested struct, single-level inheritance, multi-level inheritance
// (the base recursion re-enters), a generic struct field (the substituted `getType`
// path), a resource field that is not first (the field loop must continue past a
// non-resource field), and an array-of-struct (the array arm composed with the
// struct arm).

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

// Multi-level inheritance: the descriptor is in the deepest base, reached by the
// base recursion re-entering the struct branch more than once.
struct L2
{
    Texture2D<float4> tex;
};
struct L1 : L2
{
};
struct L0 : L1
{
};

// Generic struct field: the field type `T` is only a resource after substitution,
// which is the sole reason field types are resolved via `getType(astBuilder, ...)`.
struct Wrapper<T>
{
    T tex;
};

// Resource field that is not first: the field loop must skip the plain-data fields
// and keep going rather than short-circuiting to "not compatible".
struct Padded
{
    float4 a;
    float4 b;
    Texture2D<float4> tex;
};

// Element struct for the array-of-struct case.
struct Elem
{
    Texture2D<float4> tex;
};

RWStructuredBuffer<float4> outputBuffer;

[numthreads(1, 1, 1)]
[shader("compute")]
void main(
    [[vk::binding(/* binding */ 2, /* set */ 1)]] uniform Resources resources,
    [[vk::binding(/* binding */ 0, /* set */ 3)]] uniform Outer outer,
    [[vk::binding(/* binding */ 0, /* set */ 5)]] uniform Derived derived,
    [[vk::binding(/* binding */ 0, /* set */ 6)]] uniform L0 multi,
    [[vk::binding(/* binding */ 0, /* set */ 7)]] uniform Wrapper<Texture2D<float4> > wrapped,
    [[vk::binding(/* binding */ 0, /* set */ 8)]] uniform Padded padded,
    [[vk::binding(/* binding */ 0, /* set */ 9)]] uniform Elem elems[2])
{
    float2 uv = float2(0.0, 0.0);
    int3 at = int3(0, 0, 0);
    outputBuffer[0] = resources.tex.SampleLevel(resources.samp, uv, 0.0) +
                      outer.inner.tex.SampleLevel(outer.samp, uv, 0.0) +
                      derived.tex.SampleLevel(derived.samp, uv, 0.0) +
                      multi.tex.Load(at) +
                      wrapped.tex.Load(at) +
                      padded.tex.Load(at) + padded.a +
                      elems[0].tex.Load(at);
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

// Multi-level inheritance L0:L1:L2: tex from the deepest base at (binding 0, set 6);
// the two `base_` segments confirm the base recursion re-entered more than once.
// CHECK-DAG: OpDecorate %entryPointParams_multi_base_base_tex Binding 0
// CHECK-DAG: OpDecorate %entryPointParams_multi_base_base_tex DescriptorSet 6

// Generic struct field: `Wrapper<Texture2D<float4>>.tex` at (binding 0, set 7).
// CHECK-DAG: OpDecorate %entryPointParams_wrapped_tex Binding 0
// CHECK-DAG: OpDecorate %entryPointParams_wrapped_tex DescriptorSet 7

// Resource field not first: `Padded.tex` (after two float4 fields) at (binding 0, set 8).
// CHECK-DAG: OpDecorate %entryPointParams_padded_tex Binding 0
// CHECK-DAG: OpDecorate %entryPointParams_padded_tex DescriptorSet 8

// Array-of-struct: `Elem[2].tex` at (binding 0, set 9).
// CHECK-DAG: OpDecorate %entryPointParams_elems_tex Binding 0
// CHECK-DAG: OpDecorate %entryPointParams_elems_tex DescriptorSet 9
