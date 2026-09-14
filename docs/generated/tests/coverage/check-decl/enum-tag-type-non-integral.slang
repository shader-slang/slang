//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T12:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#failure-modes
//META: doc_section_digest=ba8996c1b140750bb2e807ee33d646c0a62d626b5b41e86bf40fe00640f5b270
//META: purpose=Verifies an enum declared with a non-integer, non-interface tag type (`float`) is rejected with E32000.
//META: intent=characterization
//META: covers=source/slang/slang-check-decl.cpp
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization: the enum-decl check validates the explicit tag type
// listed in the base list. A `float` tag drives the invalid-tag-type
// branch. CHECK pins the observed E32000 and the `'float'` substitution.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute

enum E : float { A, B }
//CHECK: ^^^^^ invalid enum tag type
//CHECK: ^^^^^ invalid tag type for 'enum': 'float'

[numthreads(1, 1, 1)]
void main() {}
