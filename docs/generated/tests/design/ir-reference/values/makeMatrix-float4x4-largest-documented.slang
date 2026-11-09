//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#aggregate-constructors
//META: doc_section_digest=81cb7db040259e3ef92a1ca250fb9ea0adfaa31cb83daa0c083596c98a915a16
//META: purpose=Stress: a `float4x4(...)` constructor with sixteen `uniform` float operands lowers to a single `makeMatrix(...)` opcode with sixteen variadic operands — pattern-bloat probe for the largest documented matrix dimension.
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The smoke test exercises `float3x3` (9 operands). This stress
// pattern takes the largest documented matrix shape — `float4x4`
// — exercising the makeMatrix opcode's variadic operand list at
// 16 components.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

uniform float fa;
uniform RWStructuredBuffer<float4x4> mbuf;

[numthreads(1, 1, 1)]
void main()
{
    mbuf[0] = float4x4(fa, fa, fa, fa, fa, fa, fa, fa, fa, fa, fa, fa, fa, fa, fa, fa);
}

// IR-LABEL: func %main
// IR: makeMatrix(%fa, %fa, %fa, %fa, %fa, %fa, %fa, %fa, %fa, %fa, %fa, %fa, %fa, %fa, %fa, %fa)
