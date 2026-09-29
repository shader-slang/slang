//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=58dd3c94958310037f5341bb94b3c7ae70279bbddba37ac9e94993785780dfa5
//META: purpose=Fires diagnostic E30070 (unsized-member-must-appear-last) - unsized member must be last
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30070
//META: catalog_name=unsized-member-must-appear-last
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// An unsized member has no fixed size, so nothing can be laid out after it. A
// second member declared behind `int a[]` triggers the ordering error, which
// the CHECK pins.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
struct S { int a[]; int b; }
void main() { S s; }

//CHECK: E30070
