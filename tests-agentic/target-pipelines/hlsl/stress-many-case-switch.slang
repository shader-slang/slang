//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/llm-generated/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=03d6fe7b8f4502160d35cd7e0f89a3145a7dfe7a6680c44cb1092265f9fe4de7
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
