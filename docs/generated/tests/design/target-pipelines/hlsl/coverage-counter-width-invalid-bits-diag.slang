//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-a-link-and-entry-point-prep
//META: doc_section_digest=d0dd806e881cf8d0bf251730509f805647fcb23a653f4914d05a97f630cbc3c6
//META: purpose=Negative: the coverage counter width is spelled `-trace-coverage-counter-width` and takes bits, and the CLI parser rejects any bit count other than 32 or 64 with E45113 before the value can reach the instrumentation pass.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the front half of the two-diagnostic split around the
// coverage counter width. The option is documented as taking *bits*
// while the pass it feeds stores *bytes*, and the bit count is
// screened at the command line — which is why the pass-side
// invalid-byte-width error is unreachable from `slangc`. Passing
// `16` exercises exactly that screen: the CHECK pins the stable
// error code and the option name, so a rename of either is caught.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute -trace-coverage-counter-width 16
//CHECK: E45113
//CHECK: accepts only `32` or `64`

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}
