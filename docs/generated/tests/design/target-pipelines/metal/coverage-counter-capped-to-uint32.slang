//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#metal-caps-coverage-counters-to-32-bits
//META: doc_section_digest=bdf089cd96aae34d768b3ab1a8777280f191af87dae6923fd22a599aad29aeaa
//META: purpose=With counting-mode coverage on Metal the synthesized `__slang_coverage` buffer is a `uint device*` and each slot is bumped by a 32-bit `atomic_fetch_add_explicit`.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the Metal counter-width cap: MSL has no 64-bit atomic
// fetch-add, so counting-mode coverage instrumentation must emit 32-bit
// counters. Compiling with `-trace-coverage` (no explicit width, i.e. the
// uncapped default of 8 bytes) and CHECKing that the synthesized coverage
// buffer parameter is `uint device*` and that the increment goes through
// `atomic_uint device*` proves the clamp to 4 bytes happened; a 64-bit
// counter would surface as `ulong` / `atomic_ulong` instead.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute -trace-coverage

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = float(tid.x);
}

// CHECK: uint device* _slang_coverage_{{[0-9]+}}
// CHECK: atomic_fetch_add_explicit
// CHECK-SAME: atomic_uint device*
// CHECK-NOT: atomic_ulong
