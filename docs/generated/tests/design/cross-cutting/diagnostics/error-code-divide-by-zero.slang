//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:32:51Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#error-code-namespace
//META: doc_section_digest=5ef206950ef0e6ecfe2a9d8abf02d78b471d242f0a4a1fb9094dec1de2c240ba
//META: purpose=A constant divide-by-zero renders the checker-range code E30002.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pin `divide-by-zero` (30002) to its rendered code. This is the
// semantic-check stage divide-by-zero, distinct from the
// preprocessor-stage one (15202). The lua entry has no per-span
// message, so we FileCheck the SIMPLE compile output for both
// the E30002 code line and the "divide by zero" header line.

//TEST:SIMPLE(filecheck=CHECK,exit-code=-1):-target hlsl -entry main -stage compute
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int a = int(tid.x);
    int b = a / 0;
}

// CHECK: E30002
// CHECK: divide by zero
