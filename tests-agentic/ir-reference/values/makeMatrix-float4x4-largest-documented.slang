//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/llm-generated/ir-reference/values.md#aggregate-constructors
//META: doc_section_digest=3e412bb1b8df51b023821b69be767e4f040104ad482e9063949a7dd98d9eda38
//META: purpose=Stress: a `float4x4(...)` constructor with sixteen `uniform` float operands lowers to a single `makeMatrix(...)` opcode with sixteen variadic operands — pattern-bloat probe for the largest documented matrix dimension.
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The smoke test exercises `float3x3` (9 operands). This stress
// pattern takes the largest documented matrix shape — `float4x4`
// — exercising the makeMatrix opcode's variadic operand list at
// 16 components.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float fa;
uniform RWStructuredBuffer<float4x4> mbuf;

[numthreads(1, 1, 1)]
void main()
{
    mbuf[0] = float4x4(fa, fa, fa, fa, fa, fa, fa, fa, fa, fa, fa, fa, fa, fa, fa, fa);
}

// IR-LABEL: func %main
// IR: makeMatrix(%fa, %fa, %fa, %fa, %fa, %fa, %fa, %fa, %fa, %fa, %fa, %fa, %fa, %fa, %fa, %fa)
