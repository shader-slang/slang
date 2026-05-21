//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/llm-generated/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=03d6fe7b8f4502160d35cd7e0f89a3145a7dfe7a6680c44cb1092265f9fe4de7
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
