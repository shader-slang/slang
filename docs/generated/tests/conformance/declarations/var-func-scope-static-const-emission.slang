//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:38+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/declarations.md#variables
//META: doc_section_digest=98b79b7c34fc8faf47b9b15881a4299669d58061046761d5503ddf01389e9d47
//META: purpose=A function-scope `static const` variable behaves like a global constant but is visible only within the function; its value folds at compile-time.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says: "A variable declared at function scope and marked with both
// `static` and `const` is a _function-scope constant_.  Semantically, a
// function-scope constant behaves like a global constant except that its name
// is only visible in the local scope."

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute

RWStructuredBuffer<float> output;
[numthreads(1, 1, 1)]
void computeMain(uint3 id : SV_DispatchThreadID)
{
    static const float LOCAL_K = 2.71828;
    output[0] = LOCAL_K * 2.0;
}

// CHECK: 5.43656
