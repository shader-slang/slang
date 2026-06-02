//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:38+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/declarations.md#variables
//META: doc_section_digest=98b79b7c34fc8faf47b9b15881a4299669d58061046761d5503ddf01389e9d47
//META: purpose=A `static const` global variable is a global constant whose value folds to a compile-time constant in emitted HLSL.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says: "A variable declared at global scope and marked with `static`
// and `const` is a _global constant_."  The value must be a compile-time
// constant expression, and the compiler folds it when emitting.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute

static const int LIMIT = 256;
RWStructuredBuffer<int> output;
[numthreads(1, 1, 1)]
void computeMain(uint3 id : SV_DispatchThreadID)
{
    output[0] = LIMIT;
}

// CHECK: 256
