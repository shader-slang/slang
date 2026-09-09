//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#compatibility-and-hlsl-storage-class-modifiers
//META: doc_section_digest=5bcb788bb16e79fc81babc467bf58625410f93828b43732190bf907b4e20e726
//META: purpose=The `precise` modifier on a local variable survives to emitted HLSL.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies `precise` is carried through to the back end instead of being
// consumed by the front end. The annotated local feeds a buffer store so it
// cannot be eliminated, and CHECK asserts the emitted HLSL declaration still
// carries `precise` -- the qualifier a downstream compiler needs in order to
// suppress re-association of the expression.

// GLSL keeps the qualifier under the same spelling, so the GLSL prefix shows
// the modifier is not an HLSL-only passthrough.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute

RWStructuredBuffer<float> outBuf;

[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    precise float a = float(tid.x) * 2.0f + 1.0f;
    outBuf[tid.x] = a;
}

//CHECK: precise float

//GLSL: precise float a_{{[0-9]+}} =
