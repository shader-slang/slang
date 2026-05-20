//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=d245d3be9942f0847c45c138b4c2780e7df19ecb
//META: doc_ref=docs/llm-generated/cross-cutting/diagnostics.md#anatomy-of-a-legacy-diagnostic
//META: doc_section_digest=04e6b41a5ed7b7f40deee827fed8dac1f618dc77bdfc81ebd67f98a5810bb953
//META: purpose=Boundary: an error diagnostic with a companion `note` helper renders the primary error code followed by a `note:` line on a separate location — the diagnostic chain.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's "anatomy of a legacy diagnostic" example shows a
// primary `err(...)` plus a `note(...)` companion. Probe the
// chained-diagnostic boundary: an error line followed by a
// physically separate `note:` line on the previous definition.
// This pins the multi-record rendering (error + note) which is
// distinct from the single-record cases tested elsewhere.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

int dup;
int dup;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID) {}

// CHECK: error[E30200]
// CHECK: declaration of 'dup' conflicts with existing declaration
// CHECK: note: see previous declaration of 'dup'
