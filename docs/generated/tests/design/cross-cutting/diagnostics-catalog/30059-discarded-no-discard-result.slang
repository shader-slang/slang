//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=7bec55f40c611ddcacda7ab9f9507c6bda6d60e263f2b9d27801d925db0f08b6
//META: purpose=Fires diagnostic E30059 (discarded-no-discard-result) - result of '[NoDiscard]' function is discarded
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30059
//META: catalog_name=discarded-no-discard-result
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `[NoDiscard]` marks a result the caller must consume. Calling the function as
// a bare expression statement discards it, and the CHECK pins the
// discarded-result error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
[NoDiscard] int f() { return 1; }
void main() { f(); }

//CHECK: E30059
