//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T12:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#failure-modes
//META: doc_section_digest=ba8996c1b140750bb2e807ee33d646c0a62d626b5b41e86bf40fe00640f5b270
//META: purpose=Verifies an extension whose base list names a non-interface struct type is rejected with E30813.
//META: intent=characterization
//META: covers=source/slang/slang-check-decl.cpp
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization: an `extension T : Base` may only add interface
// conformances, so a struct base drives the extension-inheritance check.
// CHECK pins E30813 and the `'B'` substitution.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute

struct A {}
struct B {}
extension A : B {}
//CHECK:      ^ extension cannot inherit from non-interface type 'B'

[numthreads(1, 1, 1)]
void main() {}
