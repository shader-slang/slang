//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#anatomy-of-a-diagnostic-entry
//META: doc_section_digest=79bb0fc1ac8583fef268d4e89fe89d0e6aa8ce66124dff7c481098ca077adbae
//META: purpose=Boundary: an error entry with a companion note renders the primary error code and a separately-located `note` record.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that an entry whose primary span is followed by a note renders as
// two separate records: the error at the offending declaration and a note at
// the earlier one. Two globals named `dup` are declared; CHECK asserts both
// the `error[E30200]` line and a `note:` line naming the previous declaration,
// which only appear together if the note companion survives rendering.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

int dup;
int dup;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID) {}

// CHECK: error[E30200]
// CHECK: declaration of 'dup' conflicts with existing declaration
// CHECK: note: see previous declaration of 'dup'
