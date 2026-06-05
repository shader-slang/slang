//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/generated/design/ir-reference/values.md#conversions
//META: doc_section_digest=e98041c33a94ddbbb2f967b2078765fb561c2b5e826bdd9ca046a44763e172fe
//META: purpose=Boundary: precision-narrowing `float -> half` lowers to a single `floatCast(%fa)` opcode.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc claims `floatCast` covers precision changes in both
// directions. This boundary probe isolates the narrowing edge:
// 32-bit `float` to 16-bit `half`. The widening case (float to
// double) lives in the smoke test.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float fa;
uniform RWStructuredBuffer<half> hbuf;

[numthreads(1, 1, 1)]
void main()
{
    hbuf[0] = half(fa);
}

// IR-LABEL: func %main
// IR: floatCast(%fa)
