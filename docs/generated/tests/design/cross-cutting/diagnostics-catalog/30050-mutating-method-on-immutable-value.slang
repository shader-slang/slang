//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=cf861f94d6204cb873d289653ce7c72d796755fc36030bfeebe43f1b4a1844a0
//META: purpose=Fires diagnostic E30050 (mutating-method-on-immutable-value) - mutating method cannot be called on immutable value
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30050
//META: catalog_name=mutating-method-on-immutable-value
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A `[mutating]` method may modify `this`, so it may only be called on a
// mutable value. Calling it on a `const` local is rejected, and the CHECK pins
// that error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
struct S { int a; [mutating] void m() { a = 1; } }
void main() { const S s = {1}; s.m(); }

//CHECK: E30050
