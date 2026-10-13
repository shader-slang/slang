//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#error-code-namespace
//META: doc_section_digest=5ef206950ef0e6ecfe2a9d8abf02d78b471d242f0a4a1fb9094dec1de2c240ba
//META: purpose=An unrecognised numeric id passed to `-warnings-disable` is silently ignored so a build script can name an id this compiler version does not have.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the deliberate asymmetry between the numeric and by-name paths. An id
// that no entry claims is passed to `-warnings-disable`; CHECK-NOT asserts no
// unknown-diagnostic error is raised, and CHECK asserts the unrelated
// comma-operator warning still fires, so the request was ignored rather than
// treated as a wildcard.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute -warnings-disable 987654

RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    int x = (1, 2);
    buf[0] = x;
}

// CHECK-NOT: E31111
// CHECK: E41024
// CHECK: comma operator used in expression
