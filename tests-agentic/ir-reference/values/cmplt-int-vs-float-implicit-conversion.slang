//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/llm-generated/ir-reference/values.md#conversions
//META: doc_section_digest=e98041c33a94ddbbb2f967b2078765fb561c2b5e826bdd9ca046a44763e172fe
//META: purpose=Expansion: comparing `int < float` triggers an implicit `castIntToFloat` on the int operand before `cmpLT`; verifies the documented conversion opcode bridges the type mismatch.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The Conversions table is the only place that names how mixed
// int/float operands meet a common type. The implicit conversion
// surfaces a `castIntToFloat` before the comparison.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int ia;
uniform float fa;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = select(ia < fa, 1, 0);
}

// IR-LABEL: func %main
// IR: castIntToFloat(%ia)
// IR: cmpLT(%{{[0-9]+}}, %fa)
