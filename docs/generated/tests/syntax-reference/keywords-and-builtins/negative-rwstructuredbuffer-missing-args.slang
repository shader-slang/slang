//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:30:00+00:00
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#core-module-supplied-vocabulary
//META: doc_section_digest=a84f2b8e553f246fd0e2611d7bd357de1ceb312fa259d862c4056388b6d6a68a
//META: purpose=Negative: `RWStructuredBuffer` is supplied by `hlsl.meta.slang` as a generic type and must be instantiated with a template argument; using the bare name rejects with `E30408`.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Probes the lower edge of the documented `RWStructuredBuffer<T>`
// template-arg requirement. The bare name without `<T>` triggers a
// type-checker error.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

RWStructuredBuffer outBuf;
// CHECK: error
// CHECK: RWStructuredBuffer
// CHECK: E30408

void main() {}
