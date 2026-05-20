//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/llm-generated/ir-reference/values.md#arithmetic-and-bitwise
//META: doc_section_digest=300bbe3c39d84a3dd7997f5a048fb4bd6e896b95057fa5fb8b7116c32840a6b7
//META: purpose=Integer `%` lowers to `irem`, floating-point `%` lowers to `frem`, and prefix `-` lowers to `neg`.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's Arithmetic and bitwise table lists three opcodes the
// cross-cutting bundle does not cover: `irem` (integer remainder,
// *not* modulus per the Lua note), `frem` (floating-point
// remainder), and `neg` (unary `-`).

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform int b;
uniform float fa;
uniform float fb;
uniform RWStructuredBuffer<int> buf;
uniform RWStructuredBuffer<float> fbuf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = a % b;
    fbuf[0] = fa % fb;
    buf[1] = -a;
}

// IR-LABEL: func %main
// IR: irem(%a, %b)
// IR: frem(%fa, %fb)
// IR: neg(%a)
