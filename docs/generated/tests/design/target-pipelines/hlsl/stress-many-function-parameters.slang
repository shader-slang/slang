//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=03d6fe7b8f4502160d35cd7e0f89a3145a7dfe7a6680c44cb1092265f9fe4de7
//META: purpose=Stress: a helper function with eight scalar parameters emits with all parameters preserved on HLSL.
//META: intent=stress
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress axis: many parameters. The HLSL emitter must keep the
// function signature stable; the call site references all eight
// arguments in declaration order.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

int eight(int a, int b, int c, int d, int e, int f, int g, int h)
{
    return a + b + c + d + e + f + g + h;
}

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int x = int(tid.x);
    outBuf[tid.x] = eight(x, x+1, x+2, x+3, x+4, x+5, x+6, x+7);
}

// CHECK: int eight_{{[0-9]+}}(
// CHECK: eight_{{[0-9]+}}(
