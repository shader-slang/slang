//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#inlining-and-call-graph
//META: doc_section_digest=0c02024bf756789f5e2a8e8518524bff844c5a9c7658e631aab916c9a49f2d22
//META: purpose=Nothing is emitted for the inferred ReadNone / NoSideEffect decorations themselves; their footprint is that an unused call to an inferred-pure function becomes dead code.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the stated footprint of the function-property inference by
// contrasting the two sides it decides between, inside one shader. Both
// helpers are called once and neither result is read. `pureHelper` reads
// and writes nothing outside itself, so the inference should mark it and
// its unused call becomes dead code; `impureHelper` writes a buffer, so
// the same DCE has to keep it. The CHECKs assert the pure helper and its
// call vanish entirely while the impure one and its call survive — a
// contrast that only holds if the properties were inferred rather than
// assumed.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main

RWStructuredBuffer<int> outBuf;
RWStructuredBuffer<int> sideBuf;

int pureHelper(int x)
{
    return x * 3 + 1;
}

int impureHelper(int x)
{
    sideBuf[0] = x;
    return x;
}

[numthreads(1, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    int unusedPure = pureHelper(int(tid.x));
    int unusedImpure = impureHelper(int(tid.x));
    outBuf[0] = int(tid.x);
}

// The side-effecting helper and its unused call are kept ...
// HLSL-DAG: int impureHelper_{{[0-9]+}}(int
// HLSL-DAG: impureHelper_{{[0-9]+}}(int(tid
// ... while the inferred-pure one is gone from the module entirely.
// HLSL-NOT: pureHelper
