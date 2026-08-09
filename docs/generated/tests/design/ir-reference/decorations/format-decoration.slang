//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#interpolation-and-shader-io
//META: doc_section_digest=9dcc3971ffe320daffa3155aa97909cc164f7f1f53867e82746329293f4841f5
//META: purpose=A [format("...")] attribute on a UAV global lowers to a format decoration carrying the image format as a constant operand.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `format` row: the image format requested for a UAV is
// recorded as a decoration with a single constant operand. The attribute
// takes a string in source but the dump prints an integer tag, so the CHECK
// matches `[format(N : Int)]` with the tag wildcarded - the mapping from
// format name to tag is not published, which is recorded as a doc gap.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

[format("rgba32f")]
uniform RWTexture2D<float4> tex;

uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = float(tid.x) + tex[uint2(0, 0)].x;
}

// CHECK: [format({{[0-9]+}} : Int)]
