//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#error-code-namespace
//META: doc_section_digest=c07669430fa1f58c4db53ae38c50d1523f8399ab4e7aa698054222ac63e9d41b
//META: purpose=Negative: an unrecognised diagnostic name passed to `-warnings-disable` is reported as E31111 with the offending spelling interpolated.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the by-name path fails loudly rather than being ignored. A name
// that is in no catalog is passed to `-warnings-disable`; CHECK asserts the
// unknown-diagnostic error is raised and that the rejected spelling is quoted
// back in the record's message, which distinguishes this path from the
// silently-ignored numeric one in warnings-disable-unknown-id-ignored.slang.
// Machine-readable rendering is used because the interpolated span text of a
// locationless diagnostic is not shown by the caret renderer.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute -enable-machine-readable-diagnostics -warnings-disable no-such-diagnostic-name

RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    int x = (1, 2);
    buf[0] = x;
}

// CHECK: E31111
// CHECK: unknown diagnostic 'no-such-diagnostic-name'
