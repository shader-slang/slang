//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/llm-generated/ir-reference/values.md#conversions
//META: doc_section_digest=e98041c33a94ddbbb2f967b2078765fb561c2b5e826bdd9ca046a44763e172fe
//META: purpose=Boundary: same-width signed-to-unsigned `int -> uint` lowers to `intCast(%a)`, even though no bit-width change is involved — the `intCast` opcode covers the signedness-flip step.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Same-width but signedness-changing conversion. The doc lumps
// signedness flips under `intCast`; this probe verifies that even
// the zero-bit-width-change case still surfaces the opcode (the
// IR uses the result type to encode the new signedness).

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<uint> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = uint(a);
}

// IR-LABEL: func %main
// IR: intCast(%a)
