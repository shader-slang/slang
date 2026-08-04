//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=619da2387b17d1b514e8cb1b82a03f336653b7502413b2205a6fc4753c4eeacd
//META: purpose=Stress: five-deep nested for-loops are emitted as five nested HLSL `for(...)` statements by the shared C-like emitter.
//META: intent=stress
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress axis: five-deep control-flow nesting. Each level must
// produce its own `for(...)` header; the emitter must not
// merge or unroll them at the HLSL text level (DXC handles
// optimization).

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int acc = 0;
    int bound = int(tid.x) + 1;
    for (int i = 0; i < bound; i = i + 1)
        for (int j = 0; j < bound; j = j + 1)
            for (int k = 0; k < bound; k = k + 1)
                for (int l = 0; l < bound; l = l + 1)
                    for (int m = 0; m < bound; m = m + 1)
                        acc = acc + 1;
    outBuf[tid.x] = acc;
}

// CHECK: for(
// CHECK: for(
// CHECK: for(
// CHECK: for(
// CHECK: for(
