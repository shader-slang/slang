//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=4e5f7ab56dedeffe7b1e374800617516f1d9e429f63231ad5fa4dd15419d06d1
//META: purpose=Expansion: a Slang `continue` inside a for-loop is emitted as a recognisable continue-equivalent in HLSL.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The IR `continue` is lowered to a branch to the loop's
// continue-block; the emitter renders a structured HLSL loop
// with an `if` filter so the unfiltered iterations skip.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int acc = 0;
    for (int i = 0; i < 6; i = i + 1)
    {
        if (i == 3) continue;
        acc = acc + i;
    }
    outBuf[tid.x] = acc;
}

// CHECK: for(
// CHECK: if(
