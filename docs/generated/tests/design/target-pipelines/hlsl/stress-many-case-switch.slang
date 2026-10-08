//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=c7c97cf6f5a8dc2bac27a37c7292d84bc70e3e1764ddec58018711cf9fd606ce
//META: purpose=Stress: a switch with eight case arms emits as a single HLSL `switch` with all arms preserved.
//META: intent=stress
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress axis: many switch arms. The emitter must produce a
// single `switch` block with eight `case` labels plus default,
// not a chain of `if` statements.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int x;
    switch (int(tid.x))
    {
    case 0: x = 0; break;
    case 1: x = 10; break;
    case 2: x = 20; break;
    case 3: x = 30; break;
    case 4: x = 40; break;
    case 5: x = 50; break;
    case 6: x = 60; break;
    case 7: x = 70; break;
    default: x = -1; break;
    }
    outBuf[tid.x] = x;
}

// CHECK: switch(
// CHECK-DAG: case int(0)
// CHECK-DAG: case int(7)
// CHECK: default
