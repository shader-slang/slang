//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T12:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=a3ca128124f08a4f3b66630a5a4d456463e6427c89255506f4853b27f329677e
//META: purpose=Verifies a method added to a struct via `extension` is found by member lookup and its call lowers to HLSL.
//META: intent=characterization
//META: covers=source/slang/slang-check-decl.cpp
//META: pipeline_stage=check
//META: characterization-unverified=true
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization: the extension-decl check registers `doubled()` as a
// member of `Foo` so member lookup resolves `f.doubled()`. We observe the
// generated `doubled` function being called in the emitted HLSL.
// characterization-unverified: the emitted function name is observed, not
// spec-defined.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

struct Foo { int x; }
extension Foo { int doubled() { return x * 2; } }
RWStructuredBuffer<int> o;

[numthreads(1, 1, 1)]
void main()
{
    Foo f;
    f.x = 21;
    o[0] = f.doubled();
    // CHECK: doubled{{.*}}(
}
