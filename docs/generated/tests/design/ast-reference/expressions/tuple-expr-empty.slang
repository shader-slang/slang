//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=0b0dfab9f13850a71e13b9ce6f8fb9a433ddb72267a526bf6454583638151a5b
//META: purpose=Verifies that the empty tuple spelling `()` is accepted as an expression and produces a value with no members.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `()` is the zero-member end of the tuple construction form. The empty tuple
// is bound to a local and the entry point stores a marker constant afterwards;
// every target's CHECK finding that constant proves the empty spelling was
// accepted and lowered rather than rejected as an empty parenthesized
// expression.

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
    let e = ();
    outputBuffer[0] = 7;
    // HLSL: = int(7)
    // GLSL: = 7
    // SPV: OpConstant %int 7
    // METAL: = int(7)
    // WGSL: = i32(7)
    // CUDA: = int(7)
    // CPP: = int(7)
}
