// Regression for shader-slang/slang#10668.
// `-fvk-bind-globals B S` implements DXC SPIR-V Example 3 semantics: it relocates only the synthesized
// `$Globals` uniform buffer to (set S, binding B). Resources split out of the module-scope `uniform`
// globals (here the samplers inside `Tex2D`/`Nested`) stay in the set they were originally allocated in
// (set 0) with their normal auto-assigned bindings, instead of inheriting the relocated globals set
// (which previously produced a descriptor-set/binding conflict).
//
// The expected binding values below (smp = 0, nested.deep = 1) are exactly the set-0 bindings these
// samplers receive WITHOUT the flag — in declaration order — since with the flag `$Globals` no longer
// occupies a slot in set 0. If globals-collection order ever changes, update these to match.

//TEST:SIMPLE(filecheck=SET1):-target spirv-asm -entry fragmentMain -stage fragment -emit-spirv-directly -fvk-bind-globals 0 1
//TEST:SIMPLE(filecheck=SET2):-target spirv-asm -entry fragmentMain -stage fragment -emit-spirv-directly -fvk-bind-globals 3 2
//TEST:SIMPLE(filecheck=SET0):-target spirv-asm -entry fragmentMain -stage fragment -emit-spirv-directly -fvk-bind-globals 5 0
//TEST:SIMPLE(filecheck=GLSL):-target glsl -profile ps_5_0 -entry fragmentMain -stage fragment -fvk-bind-globals 0 1 -line-directive-mode none

struct Tex2D
{
    Sampler2D smp;
    float4 invDim;
}

struct Nested
{
    Sampler2D deep;
    float4 pad;
}

uniform Tex2D sTD2DInputs[1];
uniform Nested nested;
uniform float scale;

float4 fragmentMain(float2 uv : TEXCOORD0) : SV_Target
{
    return sTD2DInputs[0].smp.Sample(uv) * scale + nested.deep.Sample(uv);
}

// `-fvk-bind-globals 0 1`: $Globals moves to set 1; both split-out samplers stay in set 0 at their
// auto-assigned bindings (0 and 1). Distinct sets means no conflict.
//SET1-DAG: OpDecorate %globalParams_sTD2DInputs_smp Binding 0
//SET1-DAG: OpDecorate %globalParams_sTD2DInputs_smp DescriptorSet 0
//SET1-DAG: OpDecorate %globalParams_nested_deep Binding 1
//SET1-DAG: OpDecorate %globalParams_nested_deep DescriptorSet 0
//SET1-DAG: OpDecorate %globalParams Binding 0
//SET1-DAG: OpDecorate %globalParams DescriptorSet 1

// `-fvk-bind-globals 3 2`: $Globals moves to set 2 / binding 3; split-out samplers stay in set 0 at
// bindings 0 and 1 -- NOT binding 3 (guards against composing the container binding onto them).
//SET2-DAG: OpDecorate %globalParams_sTD2DInputs_smp Binding 0
//SET2-DAG: OpDecorate %globalParams_sTD2DInputs_smp DescriptorSet 0
//SET2-DAG: OpDecorate %globalParams_nested_deep Binding 1
//SET2-DAG: OpDecorate %globalParams_nested_deep DescriptorSet 0
//SET2-DAG: OpDecorate %globalParams Binding 3
//SET2-DAG: OpDecorate %globalParams DescriptorSet 2

// `-fvk-bind-globals 5 0` (target set 0, non-zero binding): proves the container binding is suppressed
// independently of the set -- split-out samplers keep bindings 0 and 1 while $Globals takes binding 5.
//SET0-DAG: OpDecorate %globalParams_sTD2DInputs_smp Binding 0
//SET0-DAG: OpDecorate %globalParams_sTD2DInputs_smp DescriptorSet 0
//SET0-DAG: OpDecorate %globalParams_nested_deep Binding 1
//SET0-DAG: OpDecorate %globalParams_nested_deep DescriptorSet 0
//SET0-DAG: OpDecorate %globalParams Binding 5
//SET0-DAG: OpDecorate %globalParams DescriptorSet 0

// The same fix applies on the GLSL emit path: with `-fvk-bind-globals 0 1` the $Globals block moves to
// (set 1, binding 0) while the split-out samplers keep their set-0 bindings (GLSL omits `set = 0`).
//GLSL-DAG: layout(binding = 0, set = 1){{$}}
//GLSL-DAG: uniform block_GlobalParams
//GLSL-DAG: layout(binding = 0){{$}}
//GLSL-DAG: uniform sampler2D  globalParams_sTD2DInputs_smp
//GLSL-DAG: layout(binding = 1){{$}}
//GLSL-DAG: uniform sampler2D globalParams_nested_deep
