//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=527fd64fe800dc0775c45c1a66f5c08dc1c1123bb194dabe3cf38ab38cf14da7
//META: purpose=Fires diagnostic E30069 (no-discard-on-void-function) - '[NoDiscard]' applied to a function returning 'void'
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30069
//META: catalog_name=no-discard-on-void-function
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `[NoDiscard]` is meaningless on a function that returns nothing, so the
// attribute itself is diagnosed at its declaration. The CHECK pins that code,
// distinguishing it from the discarded-result error at the call site.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
[NoDiscard] void f() {}
void main() { f(); }

//CHECK: E30069
