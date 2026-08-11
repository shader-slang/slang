//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#phase-c-metal-legalization-lowering-phi-elimination
//META: doc_section_digest=2a4c97aeaacd2a9728a83c1944ef99c7efe30fe3fe60ef075d8d47044f03eefc
//META: purpose=`legalizeArrayReturnType` is filtered out for Metal (`!isMetalTarget && !isSPIRV`), so an array-returning function survives as `array<T, N> f(...)` on Metal while HLSL rewrites it to `void` with an `out` parameter.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the filtered-out-pass claim by compiling one source to two
// targets: the pass that rewrites an array return into an `out` parameter
// runs for HLSL but is excluded for Metal, so the same function must have
// two different signatures. The `CHECK` prefix asserts Metal keeps the
// `array<int, int(4)>` return type; the `HLSL` prefix asserts the HLSL
// emit turned the function into a `void` one with an `out` parameter.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=HLSL):-target hlsl  -entry main -stage compute

int[4] makeArr(int v)
{
    int r[4] = { v, v + 1, v + 2, v + 3 };
    return r;
}

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int r[4] = makeArr(int(tid.x));
    outBuf[tid.x] = r[0] + r[1] + r[2] + r[3];
}

// CHECK: array<int, int(4)> makeArr_{{[0-9]+}}
// HLSL: void makeArr_{{[0-9]+}}
// HLSL-SAME: out
