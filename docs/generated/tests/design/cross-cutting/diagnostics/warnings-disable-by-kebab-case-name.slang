//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:32:51Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#error-code-namespace
//META: doc_section_digest=5ef206950ef0e6ecfe2a9d8abf02d78b471d242f0a4a1fb9094dec1de2c240ba
//META: purpose=Name lookup accepts the kebab-case spelling from the catalog entry, so `-warnings-disable comma-operator-used-in-expression` suppresses E41024.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the convention-insensitive half of name lookup that takes the entry's
// declared kebab-case name. The same shader as the numeric-id test is compiled
// with the kebab spelling; CHECK-NOT asserts the warning is gone and CHECK-NOT on
// E31111 asserts the name was recognised rather than rejected as unknown.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute -warnings-disable comma-operator-used-in-expression

RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    int x = (1, 2);
    buf[0] = x;
}

// CHECK-NOT: E31111
// CHECK-NOT: E41024
// CHECK-NOT: comma operator used in expression
// CHECK: void main
