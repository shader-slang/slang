// Scope guard for the shader-slang/slang#10668 fix. `-fvk-bind-globals` must relocate ONLY the
// synthesized module-scope `$Globals` uniform buffer. Resources leaked out of an explicit
// user-declared `ConstantBuffer` or `ParameterBlock` must keep composing with their own container's
// set/binding exactly as before -- the fix must not perturb those paths.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry fragmentMain -stage fragment -emit-spirv-directly -fvk-bind-globals 7 5

struct Material
{
    float4 color;
    Sampler2D s;
}

struct LooseTex
{
    Sampler2D ls;
    float4 v;
}

// Explicit set via vk::binding: the leaked sampler must still land in set 3 alongside the buffer.
[[vk::binding(2, 3)]] ConstantBuffer<Material> gExplicit;

// ParameterBlock allocates its own set; the leaked sampler must stay in that set.
ParameterBlock<Material> gBlock;

// Loose module-scope uniforms fold into `$Globals`; only this group is relocated by the flag, and its
// split-out sampler stays in set 0 (the #10668 behavior under test).
uniform LooseTex loose;
uniform float k;

float4 fragmentMain(float2 uv : TEXCOORD0) : SV_Target
{
    return gExplicit.color + gExplicit.s.Sample(uv) + gBlock.color + gBlock.s.Sample(uv) +
           loose.ls.Sample(uv) * k;
}

// Explicit ConstantBuffer at (set 3, binding 2): leaked sampler composes to set 3, binding 3 -- unchanged.
//CHECK-DAG: OpDecorate %gExplicit Binding 2
//CHECK-DAG: OpDecorate %gExplicit DescriptorSet 3
//CHECK-DAG: OpDecorate %gExplicit_s Binding 3
//CHECK-DAG: OpDecorate %gExplicit_s DescriptorSet 3

// ParameterBlock in its own set (1): leaked sampler stays in that set at binding 1 -- unchanged.
//CHECK-DAG: OpDecorate %gBlock DescriptorSet 1
//CHECK-DAG: OpDecorate %gBlock_s Binding 1
//CHECK-DAG: OpDecorate %gBlock_s DescriptorSet 1

// $Globals relocates to (set 5, binding 7); its split-out sampler stays in set 0 (#10668 fix).
//CHECK-DAG: OpDecorate %globalParams Binding 7
//CHECK-DAG: OpDecorate %globalParams DescriptorSet 5
//CHECK-DAG: OpDecorate %globalParams_loose_ls Binding 0
//CHECK-DAG: OpDecorate %globalParams_loose_ls DescriptorSet 0
