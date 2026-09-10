//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/visibility.md#interaction-with-extern-and-export
//META: doc_section_digest=33c71c5571c1da7afb2fb1595c06713d733fa0583a985fe50cddd09355ac7966
//META: purpose=An `extern` member whose parent is an `extension` is dropped by the lookup mask regardless of its visibility keyword, so a use site reports the name as not a member at all.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the lookup mask, not the visibility filter, is what removes
// `extern` extension members. `extra` carries no visibility modifier, so it
// would be `internal` and reachable from this same module; instead the use
// site reports `member not found` (E30027) rather than the
// `declaration not accessible` (E30600) a visibility rejection produces.
// The first CHECK pins the extern decl's own E31143, the second pins the
// member-not-found at the use site.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct S { int v; }

extension S { extern int extra; }
//CHECK:                 ^^^^^ no original definition for extern decl
//CHECK:                 ^^^^^ no original definition found for `extern` decl 'extra'.
// CHECK: E31143

int test(S s) { return s.extra; }
//CHECK:                 ^^^^^ member not found
//CHECK:                 ^^^^^ 'extra' is not a member of 'S'.
// CHECK: E30027
