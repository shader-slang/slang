//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:31:07+00:00
//META: source_commit=019ec0e5ddac1c14fbad141a3f03d50cf8a5e022
//META: doc_ref=docs/llm-generated/target-pipelines/hlsl.md#phase-c-hlsl-legalization-lowering-phi-elimination
//META: doc_section_digest=e8f525c4f3b39107c481bf03414c7c2ad6a1c96a430e84aae54c4f8e6a580692
//META: purpose=Boundary: a scalar `||` with a side-effecting RHS is lowered into an explicit if/else, preserving short-circuit semantics in the emitted HLSL.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: legalizeLogicalAndOr applied to `||`. DXC
// short-circuits `||` on scalars; Slang's IR rewrite makes the
// short-circuit observable as an if/else in the emitted text.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

int sideEffect(int x)
{
    outBuf[200] = x;
    return x;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    bool b = (tid.x == 0) || (sideEffect(int(tid.x)) > 0);
    outBuf[tid.x] = b ? 1 : 0;
}

// CHECK: if(
// CHECK: sideEffect_{{[0-9]+}}(
// CHECK: else
