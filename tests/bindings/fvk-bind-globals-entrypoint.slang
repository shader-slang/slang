// Scope guard for the shader-slang/slang#10668 fix. Entry-point `uniform` parameters are collected into
// a separate synthesized group (`entryPointParams`), which `-fvk-bind-globals` never relocates. The
// #10668 fix keys on the module-scope `$Globals` wrapper parameter, so an entry-point group's split-out
// resource must be unaffected: it stays in set 0 with its normal binding even when the flag is present.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry fragmentMain -stage fragment -emit-spirv-directly -fvk-bind-globals 4 3

struct Tex2D
{
    Sampler2D smp;
    float4 invDim;
}

float4 fragmentMain(uniform Tex2D t, uniform float scale, float2 uv : TEXCOORD0) : SV_Target
{
    return t.smp.Sample(uv) * scale;
}

//CHECK-DAG: OpDecorate %entryPointParams_t_smp Binding 0
//CHECK-DAG: OpDecorate %entryPointParams_t_smp DescriptorSet 0
