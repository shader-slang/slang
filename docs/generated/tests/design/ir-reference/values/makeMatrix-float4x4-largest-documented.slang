//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ir-reference/values.md#aggregate-constructors
//META: doc_section_digest=6dd59415374c5af65d07340c8540b315850c081483ba8c4243ef7e32cac24212
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
