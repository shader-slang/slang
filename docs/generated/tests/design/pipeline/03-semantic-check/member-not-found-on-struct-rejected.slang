//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#failure-modes
//META: doc_section_digest=ba8996c1b140750bb2e807ee33d646c0a62d626b5b41e86bf40fe00640f5b270
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
