//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:43:19Z
//META: source_commit=b15b8060d2525b8cd123f2268fafbcd82e837fb3
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#phase-b-specialization-and-type-legalization
//META: doc_section_digest=8b6c1341c5d3701b416925821dd85bff877514bbcbeea56e3e6327c0e95aa563
//META: purpose=A static_assert whose condition depends on a generic parameter is checked after specialization and a satisfied assertion compiles cleanly to WGSL
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the Phase-B checkStaticAssert step evaluates static_assert
// conditions on post-specialization values: the assert's condition is
// unknowable before the generic argument N=4 is substituted, yet the
// compile succeeds and reaches WGSL emit. CHECK pins the @compute entry
// shape and the buffer store inside main, which only appear when the
// pipeline ran to completion past the assert check.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

int addBias<let N : int>(int v)
{
    static_assert(N > 0, "N must be positive");
    return v + N;
}

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = addBias<4>(int(tid.x));
}

// CHECK: @compute
// CHECK: fn main(
// CHECK: outBuf_{{[0-9]+}}
