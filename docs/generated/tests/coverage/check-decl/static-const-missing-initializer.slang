//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T12:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#failure-modes
//META: doc_section_digest=ba8996c1b140750bb2e807ee33d646c0a62d626b5b41e86bf40fe00640f5b270
//META: purpose=Verifies a `static const` variable declared without an initializer is rejected with E31225.
//META: intent=characterization
//META: covers=source/slang/slang-check-decl.cpp
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization: a module-scope `static const` declaration with no
// initializer drives the decl-check path that requires an initial value
// for compile-time-constant storage. CHECK pins the observed E31225 code
// and message text emitted by slang-check-decl.cpp.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute

static const int x;
//CHECK:         ^ missing initializer for static const
//CHECK:         ^ static const variable 'x' must have an initializer

[numthreads(1, 1, 1)]
void main() {}
