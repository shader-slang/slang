//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#loop-transformations
//META: doc_section_digest=5346083df587dd61328c503d8394df501ae34fed3523d17881e22225881336af
//META: purpose=Boundary: satcoop fusion floats the intervening instructions only as their dependencies allow, so a second saturated_cooperation call that consumes the first one's result keeps its own wave-match region.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the limit on the fusion, which is what makes the fused case
// meaningful rather than unconditional. This shader is the fused test's
// shader with one change: the second call cooperates over a value derived
// from the first call's result, so the two cannot be floated together.
// The CHECKs assert two wave-match regions survive, in contrast to the one
// the independent version produces.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main

RWStructuredBuffer<int> outBuf;

int coop1(float x, int i) { return int(x) * 2 + i; }
int fall1(float x, int i) { return int(x) * 3 + i; }
int coop2(float x, int i) { return int(x) * 4 + i; }
int fall2(float x, int i) { return int(x) * 5 + i; }

[numthreads(128, 1, 1)]
void main(uint tig: SV_GroupIndex)
{
    let v = float(tig);
    let a = saturated_cooperation(coop1, fall1, v, 1);
    let b = saturated_cooperation(coop2, fall2, float(a), 2);
    outBuf[tig] = a + b;
}

// Two separate wave-match regions, one per unfused call.
// HLSL: WaveMatch(
// HLSL: WaveMatch(
