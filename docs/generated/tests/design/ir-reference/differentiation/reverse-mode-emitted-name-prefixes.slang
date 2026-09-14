//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#reverse-mode
//META: doc_section_digest=b18c139464f823deeeb4d040a62520838e80b082fa7e2f42dee8680582a5dec9
//META: purpose=Reverse mode gives the propagate function the s_bwdProp_ prefix and the full intermediate-context struct the s_bwdCallableCtx_ prefix, so f yields s_bwdProp_f alongside a s_bwdCallableCtx_f struct.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the reverse-mode half of the generated-name rule, which is the only
// way the reverse-mode operators become visible to someone reading emitted
// code. `__bwd_diff(myfunc)` makes the passes split `myfunc` into phases, so
// the propagate phase and the struct carrying the hoisted primal state both
// have to be emitted. The first CHECK requires a struct definition named
// `s_bwdCallableCtx_myfunc_<n>` and the second a function named
// `s_bwdProp_myfunc_<n>`; asserting the struct as `struct s_bwdCallableCtx...`
// rather than as a bare substring keeps it from being satisfied by the
// per-parameter `s_paramCtx_s_bwdCallableCtx_...` wrapper the emitter also
// produces, which would leave the actual context struct unchecked.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute

uniform float a;
uniform RWStructuredBuffer<float> buf;

[Differentiable]
float myfunc(float x)
{
    return x * x + x;
}

[numthreads(1, 1, 1)]
void main()
{
    DifferentialPair<float> dp = diffPair(a, 0.0);
    __bwd_diff(myfunc)(dp, 1.0);
    buf[0] = dp.d;
}

//HLSL: struct s_bwdCallableCtx_myfunc_{{[0-9]+}}
//HLSL: s_bwdProp_myfunc_{{[0-9]+}}(
//GLSL: struct s_bwdCallableCtx_myfunc_{{[0-9]+}}
//GLSL: s_bwdProp_myfunc_{{[0-9]+}}(
//METAL: struct s_bwdCallableCtx_myfunc_{{[0-9]+}}
//METAL: s_bwdProp_myfunc_{{[0-9]+}}(
//WGSL: struct s_bwdCallableCtx_myfunc_{{[0-9]+}}
//WGSL: s_bwdProp_myfunc_{{[0-9]+}}(
//CUDA: struct s_bwdCallableCtx_myfunc_{{[0-9]+}}
//CUDA: s_bwdProp_myfunc_{{[0-9]+}}(
//CPP: struct s_bwdCallableCtx_myfunc_{{[0-9]+}}
//CPP: s_bwdProp_myfunc_{{[0-9]+}}(
