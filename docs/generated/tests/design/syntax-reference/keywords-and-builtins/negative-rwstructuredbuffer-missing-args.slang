//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#core-module-syntax-declarations
//META: doc_section_digest=12582dfa01eb81163b717e59e208875fece8e3cb827d3738a483bbc260d0c7d8
//META: purpose=Negative: `RWStructuredBuffer` is supplied by `hlsl.meta.slang` as a generic type and must be instantiated with a template argument; using the bare name rejects with `E30408`.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Probes the lower edge of the documented `RWStructuredBuffer<T>`
// template-arg requirement. The bare name without `<T>` triggers a
// type-checker error.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

RWStructuredBuffer outBuf;
// CHECK-DAG: error
// CHECK-DAG: RWStructuredBuffer
// CHECK-DAG: E30408

void main() {}
