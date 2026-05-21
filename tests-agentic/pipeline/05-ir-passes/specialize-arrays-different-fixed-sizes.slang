//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/llm-generated/pipeline/05-ir-passes.md#specialization-and-generics
//META: doc_section_digest=80bc5973053356e62dbd6f6ded27b7e93976729f33655c137440d5c954689e89
//META: purpose=Expansion: the specialize-arrays pass produces independent specializations for two different `let N` array sizes (N=2 and N=8) — both inline cleanly and no generic shape survives.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `sumN<let N : int>` is instantiated twice — once with N=2 and once
// with N=8. Each call site produces its own specialization. Because
// the helper is `[ForceInline]` it inlines away and `sumN` does not
// appear in the HLSL emit.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

[ForceInline]
int sumN<let N : int>(int data[N])
{
    int s = 0;
    [ForceUnroll] for (int i = 0; i < N; ++i) s += data[i];
    return s;
}

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int small[2] = { int(tid.x), int(tid.y) };
    int big[8] = { 0, 1, 2, 3, 4, 5, 6, int(tid.z) };
    buf[0] = sumN<2>(small) + sumN<8>(big);
}

// HLSL: void main
// HLSL-NOT: sumN
