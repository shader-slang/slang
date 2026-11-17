//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:35:46Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/name-resolution/visibility.md#edge-cases-and-failure-modes
//META: doc_section_digest=c8df5707a8529f8b54eb08117b70cc5d32a632cfac0bf45bd758aba4031e885f
//META: purpose=Distinct concrete specializations such as `extension G<int>` and `extension G<float>` do not see each other's `private` members, so the cross-extension call does not resolve (undefined identifier, E30015).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a `private` member declared in `extension G<int>` is not
// reachable from `extension G<float>`: the two extension targets are neither
// equal nor specializable to one another. The extension member is not even a
// lookup candidate from the float extension, so the call reports an undefined
// identifier rather than 30600; the annotation pins that to the call site and
// E30015 keeps the test from passing on an unrelated error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct G<T> {}

extension G<int>
{
    private int secret() { return 7; }
}

extension G<float>
{
    int reach() { return secret(); }
//CHECK:                 ^^^^^^ undefined identifier
//CHECK:                 ^^^^^^ undefined identifier 'secret'.
// CHECK: E30015
}
