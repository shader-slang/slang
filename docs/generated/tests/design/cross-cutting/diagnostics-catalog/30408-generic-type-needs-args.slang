//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=07923392cc79ee8052345335ef6b53b6de7e212653f0c69f9d63bac02366675e
//META: purpose=Fires diagnostic E30408 (generic-type-needs-args) - generic type '~type:Type' used without argument
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30408
//META: catalog_name=generic-type-needs-args
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A generic type name is not a type until it is applied to arguments. Using
// bare `S` where a type is expected fires the needs-arguments error, which the
// CHECK pins.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
struct S<T> { T v; }
void main() { S s; }

//CHECK: E30408
