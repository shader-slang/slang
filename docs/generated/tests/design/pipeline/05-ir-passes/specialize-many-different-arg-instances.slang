//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#specialization-and-generics
//META: doc_section_digest=4af16ce22c9a66bc00390c870ab8fa13bdbb68c7437a11e64d86f8f9c02cdbe4
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
