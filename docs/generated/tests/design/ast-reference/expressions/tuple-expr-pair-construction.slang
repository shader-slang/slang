//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=0b0dfab9f13850a71e13b9ce6f8fb9a433ddb72267a526bf6454583638151a5b
//META: purpose=Verifies that the parenthesized comma form `(a, b)` builds a tuple value whose members are readable positionally under the 2026 language rules.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Under the 2026 language rules `(a, b)` is a tuple construction rather than a
// comma operator. The first member is derived from the dispatch id so it stays
// symbolic while the second stays a constant, and both are stored to separate
// buffer slots. Each target's CHECK asserts that the constant member reached
// its own slot, which requires the pair to have been built and destructured by
// position.

//TEST:SIMPLE(filecheck=HLSL):-std 2026 -target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-std 2026 -target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPV):-std 2026 -target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-std 2026 -target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-std 2026 -target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-std 2026 -target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-std 2026 -target cpp -entry computeMain -stage compute

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    let t = (10 + int(tid.x), 20);
    outputBuffer[0] = t._0;
    outputBuffer[1] = t._1;
    // HLSL: = int(20)
    // GLSL: = 20
    // SPV: OpConstant %int 20
    // METAL: = int(20)
    // WGSL: = i32(20)
    // CUDA: = int(20)
    // CPP: = int(20)
}
