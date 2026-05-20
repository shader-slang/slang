//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=d245d3be9942f0847c45c138b4c2780e7df19ecb
//META: doc_ref=docs/llm-generated/pipeline/05-ir-passes.md#specialization-and-generics
//META: doc_section_digest=80bc5973053356e62dbd6f6ded27b7e93976729f33655c137440d5c954689e89
//META: purpose=Stress: the same generic called with three distinct type arguments (int / float / uint) is each specialized independently — the three concrete operand types `int ai_0`, `float af_0`, `uint au_0` survive in the HLSL GlobalParams struct.
//META: intent=stress
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `pickFirst<T>` is called three times in `main` with three different
// concrete type arguments. The specialize pass produces three
// concrete specializations; the helper is `[ForceInline]` so its name
// disappears, but the three distinct operand types remain visible in
// the cbuffer field declarations.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main

uniform int ai;
uniform float af;
uniform uint au;
uniform RWStructuredBuffer<int> buf;

[ForceInline]
T pickFirst<T>(T x, T y) { return x; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int x = pickFirst<int>(ai, ai + 1);
    float y = pickFirst<float>(af, af + 1.0f);
    uint z = pickFirst<uint>(au, au + 1u);
    buf[0] = x + int(y) + int(z);
}

// HLSL: struct GlobalParams
// HLSL-DAG: int ai
// HLSL-DAG: float af
// HLSL-DAG: uint au
// HLSL-NOT: pickFirst
