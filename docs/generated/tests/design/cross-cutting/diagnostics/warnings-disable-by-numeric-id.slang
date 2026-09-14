//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#error-code-namespace
//META: doc_section_digest=e5426f9afa17becb9e7d33c23301014e082fad12bbf8aff1c4a169d90575556e
//META: purpose=`-warnings-disable` accepts an integer id from the shared namespace and suppresses that diagnostic.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the integer-id form of the override request. The comma-operator
// warning is triggered and 41024 is passed to `-warnings-disable`; CHECK-NOT
// asserts neither the code nor the message survives, while CHECK asserts the
// entry point was still emitted so the suppression is not just a failed compile.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute -warnings-disable 41024

RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    int x = (1, 2);
    buf[0] = x;
}

// CHECK-NOT: E41024
// CHECK-NOT: comma operator used in expression
// CHECK: void main
