//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#visibility-scopes
//META: doc_section_digest=fc40943abb8d4c479b692d39171c3acf02766b831d8fe5456b3c43f7df6d3a63
//META: purpose=`private` is type-scoped rather than file-scoped, so a free function in the same file cannot read a `private` member of a struct (E30600).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the scope `private` names is the declaring type, not the source
// file. The reader is the C++-shaped intuition that "same file" grants access;
// the test puts the reader in the same file but outside the type and asserts it
// is still rejected. CHECK pins E30600 and the message naming the member.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct S
{
    private int secret;
}

int peek(S s)
{
    return s.secret;
//CHECK: E30600
//CHECK: 'secret' is not accessible from the current context.
}

void main()
{
}
