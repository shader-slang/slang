//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#interpolation-and-shader-io
//META: doc_section_digest=9dcc3971ffe320daffa3155aa97909cc164f7f1f53867e82746329293f4841f5
//META: purpose=Boundary - a `linear` modifier produces interpolationMode with mode operand 0, the lowest interpolation-mode value observed in the dump.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the `modeOperand` of `interpolationMode` really encodes
// which modifier was written. `linear` pins the literal 0 here, while the
// `nointerpolation` and `sample` siblings pin 2 and 4, so the three tests
// together show the operand is a per-modifier tag rather than a constant.
// The doc does not publish the encoding; that is recorded as a doc gap.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<float> buf;

struct V
{
    linear float u : TEXCOORD0;
};

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID, V v)
{
    buf[0] = v.u + float(tid.x);
}

// CHECK: [interpolationMode(0 : Int)]
