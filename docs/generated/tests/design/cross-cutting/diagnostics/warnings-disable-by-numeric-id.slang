//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:32:51Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#error-code-namespace
//META: doc_section_digest=5ef206950ef0e6ecfe2a9d8abf02d78b471d242f0a4a1fb9094dec1de2c240ba
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
