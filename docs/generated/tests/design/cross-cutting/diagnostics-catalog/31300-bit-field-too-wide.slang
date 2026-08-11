//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=273c89e986f24ae40d734612e749a14fd2a5062eb5fdb72f8cab67646c122a99
//META: purpose=Fires diagnostic E31300 (bit-field-too-wide) - bit-field size exceeds type width
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31300
//META: catalog_name=bit-field-too-wide
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A bit-field cannot be wider than the type that stores it. Requesting 40 bits
// from a 32-bit `int` overflows the storage, and the CHECK pins that error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -stage compute
struct S { int a : 40; }
[numthreads(1,1,1)]
void main() { S s; }

//CHECK: E31300
