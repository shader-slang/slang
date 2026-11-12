//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-c-hlsl-legalization-lowering-phi-elimination
//META: doc_section_digest=a8c4fe8f82db2627965e05b91b8cd95b5dca976c67648791b7338b2ffdef93e1
//META: purpose=Boundary: a scalar `&&` with a side-effecting RHS is lowered into an explicit if/else, preserving short-circuit semantics in the emitted HLSL.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: legalizeLogicalAndOr — DXC short-circuits on
// scalars, and Slang's IR rewrite makes the short-circuit
// observable in the emitted text by lifting the RHS into an
// if/else branch.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

int sideEffect(int x)
{
    outBuf[100] = x;
    return x;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    bool b = (tid.x > 0) && (sideEffect(int(tid.x)) > 0);
    outBuf[tid.x] = b ? 1 : 0;
}

// CHECK: if(
// CHECK: sideEffect_{{[0-9]+}}(
// CHECK: else
