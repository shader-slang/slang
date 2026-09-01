//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T12:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#failure-modes
//META: doc_section_digest=ba8996c1b140750bb2e807ee33d646c0a62d626b5b41e86bf40fe00640f5b270
//META: purpose=Verifies a class inheriting from a non-class, non-interface struct type is rejected with E30814.
//META: intent=characterization
//META: covers=source/slang/slang-check-decl.cpp
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization: a `class C : Base` base must be a class or an
// interface. A plain struct base drives the class-inheritance branch of
// the decl check. CHECK pins E30814 and the type-name substitutions.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute

struct NotIface {}
class C : NotIface {}
//CHECK:  ^^^^^^^^ class 'C' cannot inherit from type 'NotIface' that is neither a class nor an interface

[numthreads(1, 1, 1)]
void main() {}
