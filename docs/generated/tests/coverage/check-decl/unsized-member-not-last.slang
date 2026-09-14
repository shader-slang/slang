//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T12:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#failure-modes
//META: doc_section_digest=ba8996c1b140750bb2e807ee33d646c0a62d626b5b41e86bf40fe00640f5b270
//META: purpose=Verifies an unsized (runtime-array) struct member that is not the last member is rejected with E30070.
//META: intent=characterization
//META: covers=source/slang/slang-check-decl.cpp
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization: a member of unknown compile-time size (`int arr[]`)
// must be the trailing member of its composite. Placing a sized member
// after it drives the unsized-member-placement check. CHECK pins E30070.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute

struct S
{
    int arr[];
//CHECK:^^^ unsized member must be last
//CHECK:^^^ member with unknown size at compile time can only appear as the last member in a composite type.
    int b;
}

[numthreads(1, 1, 1)]
void main() {}
