//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T18:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/llm-generated/cross-cutting/ir-instructions.md#value-instructions
//META: doc_section_digest=ddd065b774b7979630d27ec19315b658865b6fb853c55bd6e6ee0a0aced77aba
//META: purpose=`float3(a, b, 1.0)` with `a : float`, `b : int` lowers to an explicit `castIntToFloat(%b)` followed by the documented `makeVector` aggregate-constructor opcode — pins mixed-precision vector composition.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's value-instructions row admits `makeVector` as the
// aggregate constructor and the conversion family for type promotion.
// A heterogeneous `float3(float, int, literal)` lowers to a
// `castIntToFloat` on the int operand and then `makeVector` of three
// `Float` operands — pinning both opcodes' roles in mixed-precision
// composition.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float a;
uniform int b;
uniform RWStructuredBuffer<float3> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = float3(a, b, 1.0);
}

// IR-LABEL: func %main
// IR: castIntToFloat(%b)
// IR: makeVector(%a, %{{.*}}, 1 : Float)
