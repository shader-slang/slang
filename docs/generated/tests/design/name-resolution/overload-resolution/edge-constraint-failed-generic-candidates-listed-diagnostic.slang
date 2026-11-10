//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:35:39Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#edge-cases-and-failure-modes
//META: doc_section_digest=b389e40b388d5f30478f28f97f1bbbbdfb2576ddc1cea6d321380e5d95ca739d
//META: purpose=Generic candidates whose inference failed on an interface constraint are listed under the no-applicable-overload error, each followed by a does-not-conform note naming its own interface.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies constraint-failed generic candidates are retained and explained.
// Two overloads of `f` constrain T to different interfaces and `int`
// satisfies neither; CHECK-DAG asserts the no-applicable error plus one
// does-not-conform note (E40022) per interface, so each candidate is
// listed rather than pruned.

//TEST:SIMPLE(filecheck=CHECK,exit-code=-1): -target hlsl -entry main -stage compute

interface I0 { int a(); }
interface I1 { int b(); }
int f<T : I0>(T x) { return 1; }
int f<T : I1>(T x) { return 2; }

void main()
{
    // CHECK-DAG: E39999
    // CHECK-DAG: E40022{{.*}}'I0'
    // CHECK-DAG: E40022{{.*}}'I1'
    int r = f(5);
}
