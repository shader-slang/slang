//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:16:11+00:00
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-c-hlsl-legalization-lowering-phi-elimination
//META: doc_section_digest=e8f525c4f3b39107c481bf03414c7c2ad6a1c96a430e84aae54c4f8e6a580692
//META: purpose=legalizeArrayReturnType rewrites T[N] foo() into void foo(out T[N]) on HLSL because DXC disallows array return values.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// DXC rejects functions returning fixed-size arrays. Slang's
// `legalizeArrayReturnType` pass (HLSL is `!isMetalTarget &&
// !isSPIRV`) rewrites the return into an out-parameter.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

int[4] foo(int x)
{
    int r[4] = { x, x + 1, x + 2, x + 3 };
    return r;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int r[4] = foo(int(tid.x));
    outBuf[tid.x] = r[1];
}

// CHECK: void foo_{{[0-9]+}}(int {{.*}}, out int{{.*}}[
