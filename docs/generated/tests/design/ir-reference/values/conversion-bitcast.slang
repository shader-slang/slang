//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#conversions
//META: doc_section_digest=15a849f7cabcf06b95570121d08d30f735912628e8a357779f17fa39de0aca01
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
