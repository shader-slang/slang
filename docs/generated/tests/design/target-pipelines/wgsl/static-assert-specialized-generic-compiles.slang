//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#phase-b-specialization-and-type-legalization
//META: doc_section_digest=d55555774e3671195c911b5700b7f3b76e600649eaa66a65a238dcf8a1620c86
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
