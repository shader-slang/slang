//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:38+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/declarations.md#variables
//META: doc_section_digest=98b79b7c34fc8faf47b9b15881a4299669d58061046761d5503ddf01389e9d47
//META: purpose=A `static` (non-`const`) global variable emits as a module-scope `static` variable in HLSL; each invocation has its own copy.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a `static` (non-const) global emits as a `static` module-scope variable in HLSL.
// `gCounter` is incremented inside the compute shader; CHECK lines assert both the `static`
// keyword and the variable name appear in the emitted HLSL.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute

static int gCounter = 0;
RWStructuredBuffer<int> output;
[numthreads(1, 1, 1)]
void computeMain(uint3 id : SV_DispatchThreadID)
{
    gCounter += 1;
    output[0] = gCounter;
}

// CHECK: static
// CHECK: gCounter
