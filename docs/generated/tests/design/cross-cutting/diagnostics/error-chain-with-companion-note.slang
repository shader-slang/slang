//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:32:51Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#anatomy-of-a-diagnostic-entry
//META: doc_section_digest=f22b494b8e869f4fe09a0366d2108166de2b12d27d47542c79c644cf98a38302
//META: purpose=Boundary: an error entry with a companion note renders the primary error code and a separately-located `note` record.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that an entry whose primary span is followed by a note renders as
// two separate records: the error at the offending declaration and a note at
// the earlier one. Two globals named `dup` are declared; CHECK asserts both
// the `error[E30200]` line and a `note:` line naming the previous declaration,
// which only appear together if the note companion survives rendering.

//TEST:SIMPLE(filecheck=CHECK,exit-code=-1):-target hlsl -entry main -stage compute

int dup;
int dup;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID) {}

// CHECK: error[E30200]
// CHECK: declaration of 'dup' conflicts with existing declaration
// CHECK: note: see previous declaration of 'dup'
