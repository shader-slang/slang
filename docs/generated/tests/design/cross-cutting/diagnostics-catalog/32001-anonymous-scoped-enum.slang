//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=5a01ebf4b8b814e06aacd10716e6b733381a92cddabef3b83b20a934f4c5bb4e
//META: purpose=Fires diagnostic E32001 (anonymous-scoped-enum) - anonymous scoped enum is not allowed
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=32001
//META: catalog_name=anonymous-scoped-enum
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A scoped enum's members are reached through its name, so an anonymous scoped
// enum would be unusable. The CHECK pins the anonymous-scoped-enum error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -stage compute
enum class { A, B };
[numthreads(1,1,1)]
void main() { }

//CHECK: E32001
