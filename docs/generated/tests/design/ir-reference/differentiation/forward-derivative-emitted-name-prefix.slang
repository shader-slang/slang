//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#reverse-mode
//META: doc_section_digest=b18c139464f823deeeb4d040a62520838e80b082fa7e2f42dee8680582a5dec9
//META: purpose=The forward-mode transcriber names the derivative it builds for a function f with the s_fwd_ prefix, so f yields s_fwd_f in every text target.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the one part of the differentiation family that a reader actually
// meets in generated code: none of the opcodes reaches target text, but the
// functions the passes build from them do, under a generated name with a fixed
// prefix. `__fwd_diff(myfunc)` forces the forward-mode derivative of `myfunc`
// to be built and called, so the derivative body has to be emitted; each CHECK
// requires a definition of `s_fwd_myfunc_<n>` in that target's text. The
// trailing `_<n>` is the emitter's ordinary disambiguating suffix, so it is
// matched with a wildcard while the prefix and the original name are pinned —
// those two are what the naming rule commits to. The fan-out is one directive
// per text-emit target because the name is produced by the emitter.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute

uniform float a;
uniform float d;
uniform RWStructuredBuffer<float> buf;

[Differentiable]
float myfunc(float x)
{
    return x * x + x;
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = __fwd_diff(myfunc)(DifferentialPair<float>(a, d)).d;
}

//HLSL: s_fwd_myfunc_{{[0-9]+}}(
//GLSL: s_fwd_myfunc_{{[0-9]+}}(
//METAL: s_fwd_myfunc_{{[0-9]+}}(
//WGSL: s_fwd_myfunc_{{[0-9]+}}(
//CUDA: s_fwd_myfunc_{{[0-9]+}}(
//CPP: s_fwd_myfunc_{{[0-9]+}}(
