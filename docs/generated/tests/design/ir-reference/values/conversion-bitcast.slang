//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ir-reference/values.md#conversions
//META: doc_section_digest=d86bedaed855595e10c45be817dcd07ae903a5905ada08a17d56c55e59daa232
//META: purpose=The `bit_cast<T>(...)` builtin lowers to the `bitCast` opcode (reinterpret bits without changing them).
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The Conversions table lists `bitCast` with AST origin "`BitCastExpr`
// and `__bit_cast` intrinsic". The Slang core-module surface is
// `bit_cast<TargetType>(value)`. A reinterpret of an int's bits as a
// float surfaces the opcode.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<float> fbuf;

[numthreads(1, 1, 1)]
void main()
{
    float g = bit_cast<float>(a);
    fbuf[0] = g;
}

// IR-LABEL: func %main
// IR: bitCast(%a)
