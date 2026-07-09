//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ir-reference/values.md#arithmetic-and-bitwise
//META: doc_section_digest=1d2f08dfecd3e2ccb26b50652f1294ebe4c1c5182a6aab140a589ccecd16faa1
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
