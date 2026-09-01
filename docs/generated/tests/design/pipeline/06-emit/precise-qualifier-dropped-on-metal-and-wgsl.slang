//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#shared-c-like-base
//META: doc_section_digest=4b10e63cd2245887946d5a04f643460694312535bff48052c288d542dac985ca
//META: purpose=`emitTempModifiers` is a per-target override: HLSL and GLSL carry a `precise` local through to the emitted text, while Metal and WGSL drop it and diagnose that the qualifier is unsupported on the target.
//META: intent=negative
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the temp-modifier divergence between backends. The same `precise`
// local is compiled to four targets: the HLSL and GLSL prefixes assert the
// qualifier reaches the emitted declaration, while the Metal and WGSL
// prefixes assert both that the declaration comes out unqualified and that the
// compiler said so, pinned by the stable diagnostic code E56005. The CHECK-NOT
// patterns include the mangled name so they cannot accidentally match the
// diagnostic's own echo of the offending source line. The
// arithmetic reads from the dispatch id so the constant folder cannot remove
// the `precise` temporary before emit.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float a = float(tid.x);
    float b = float(tid.y);
    precise float p = a * b + 1.0f;
    outBuf[0] = p;
}

// HLSL: precise float p_{{[0-9]+}}
// GLSL: precise float p_{{[0-9]+}}
// METAL-DAG: E56005
// METAL-DAG: {{^}}    float p_{{[0-9]+}} =
// METAL-NOT: precise float p_
// WGSL-DAG: E56005
// WGSL-DAG: {{^}}    var p_{{[0-9]+}} : f32 =
// WGSL-NOT: precise float p_
