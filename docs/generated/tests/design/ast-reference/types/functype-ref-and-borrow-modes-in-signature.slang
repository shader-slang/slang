//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:33:31+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/ast-reference/types.md#functype
//META: doc_section_digest=ec7fb3af8ac52fbfa85f29879864eda7df1485be04cba1defcab59284c8b6a53
//META: purpose=Each by-reference or borrow parameter mode is a distinct wrapper type inside the FuncType: borrow, ref, ref_readonly and ref_writeonly all print in the rendered function type of a candidate.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the borrow and three by-reference mode wrappers each have
// their own printed spelling in the function type (`borrow`, `ref`,
// `ref_readonly`, `ref_writeonly`), which can only be true if the mode is
// part of each parameter's type. The read-only and write-only modes are
// core-module keywords, so the test enables -experimental-feature. The call
// has no arguments so the checker prints the candidate.

//TEST:SIMPLE(filecheck=CHECK,exit-code=-1): -target hlsl -stage compute -entry main -experimental-feature

void f(__borrow int a, __ref int b, __ref_readonly int c, __ref_writeonly int d) {}

[numthreads(1, 1, 1)]
void main()
{
    f();
}
// CHECK: E39999
// CHECK: candidate: func f(borrow int, ref int, ref_readonly int, ref_writeonly int) -> void
