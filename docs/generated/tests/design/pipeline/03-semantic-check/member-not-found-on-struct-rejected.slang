//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#failure-modes
//META: doc_section_digest=8518a72edfb4aa449935321fb64c48a945528cb8e3f926bdb661166c52241bd1
//META: purpose=Member lookup on a struct without the requested name produces E30027 with the requested member and host type cited verbatim.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion of E30027 member-not-found at a struct-typed receiver
// (the existing test uses a vector swizzle host). Verify the
// diagnostic names both the missing member and the host type.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct S { int x; }

void main()
{
    S s;
    int y = s.bogus;
//CHECK:      ^^^^^ member not found
//CHECK:      ^^^^^ 'bogus' is not a member of 'S'.
// CHECK: E30027
}
