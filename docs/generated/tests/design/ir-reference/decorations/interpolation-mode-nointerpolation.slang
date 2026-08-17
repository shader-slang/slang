//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#interpolation-and-shader-io
//META: doc_section_digest=9dcc3971ffe320daffa3155aa97909cc164f7f1f53867e82746329293f4841f5
//META: purpose=Boundary - a `nointerpolation` modifier produces interpolationMode with mode operand 2, distinct from linear (0) and sample (4).
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the mid-range value of the interpolation-mode encoding. Using
// the same varying-field surface as its `linear` and `sample` siblings and
// changing only the modifier isolates the operand as the single thing under
// test.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<float> buf;

struct V
{
    nointerpolation float u : TEXCOORD0;
};

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID, V v)
{
    buf[0] = v.u + float(tid.x);
}

// CHECK: [interpolationMode(2 : Int)]
