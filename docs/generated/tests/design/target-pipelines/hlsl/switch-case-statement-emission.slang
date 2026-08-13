//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=4e5f7ab56dedeffe7b1e374800617516f1d9e429f63231ad5fa4dd15419d06d1
//META: purpose=Expansion: a Slang switch is emitted as an HLSL `switch`/`case` statement by the shared C-like emitter.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The shared CLikeSourceEmitter handles the `switch` IR
// construct natively in HLSL emit. Each `case` lowers to a
// labelled branch.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int x = 0;
    switch (int(tid.x))
    {
    case 0: x = 100; break;
    case 1: x = 200; break;
    case 2: x = 300; break;
    default: x = -1; break;
    }
    outBuf[tid.x] = x;
}

// CHECK: switch(
// CHECK: case
// CHECK: break;
// CHECK: default
