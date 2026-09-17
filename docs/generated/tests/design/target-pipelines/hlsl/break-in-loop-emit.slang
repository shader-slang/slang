//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=c7c97cf6f5a8dc2bac27a37c7292d84bc70e3e1764ddec58018711cf9fd606ce
//META: purpose=Expansion: a Slang `break` inside a loop is emitted as a `break` (or equivalent loop exit) in HLSL.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// HLSL emit preserves loop early-exit. The IR-level break is
// rendered into a `break;` statement.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int found = -1;
    for (int i = 0; i < 32; i = i + 1)
    {
        if (i == int(tid.x))
        {
            found = i * 2;
            break;
        }
    }
    outBuf[tid.x] = found;
}

// CHECK: for(
// CHECK: break;
