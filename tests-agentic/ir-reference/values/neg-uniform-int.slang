//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/llm-generated/ir-reference/values.md#arithmetic-and-bitwise
//META: doc_section_digest=300bbe3c39d84a3dd7997f5a048fb4bd6e896b95057fa5fb8b7116c32840a6b7
//META: purpose=Boundary: prefix `-` on a `uniform int` (whose value spans the full signed-32 range, including the documented MIN where unary negation overflows) lowers to `neg(%a)`.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The neg opcode is exercised in the smoke test through compound
// expressions. This boundary probe isolates `-a` on a uniform —
// the `a` operand's value is unknown to the compiler, so the
// `neg(MIN)` overflow edge is preserved as a single `neg`
// instruction. Lowering does not pre-bake the overflow path.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = -a;
}

// IR-LABEL: func %main
// IR: neg(%a)
