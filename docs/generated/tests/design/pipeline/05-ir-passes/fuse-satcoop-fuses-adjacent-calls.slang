//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#loop-transformations
//META: doc_section_digest=5346083df587dd61328c503d8394df501ae34fed3523d17881e22225881336af
//META: purpose=Adjacent calls to the core-module saturated_cooperation builtin are fused into one, so two wave-match regions in the emit collapse into one.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the fusion the satcoop pass performs, using the wave-match
// region as the countable footprint. The two `saturated_cooperation` calls
// below cooperate over the same value and neither depends on the other's
// result, so nothing blocks fusion. The CHECK asserts exactly one
// `WaveMatch` region is emitted: the `CHECK-NOT` after it fails if the two
// calls each kept their own region, which is the unfused shape.

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
    let b = saturated_cooperation(coop2, fall2, v, 2);
    outBuf[tig] = a + b;
}

// Exactly one wave-match region survives for the two fused calls.
// HLSL: WaveMatch(
// HLSL-NOT: WaveMatch(
