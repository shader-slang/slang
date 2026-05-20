//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/llm-generated/architecture/overview.md#compilation-request-lifecycle
//META: doc_section_digest=4dae689ab71645e4934f3d5e29afb21da2d6bcdf308ea84e96e25981a6c6dd7b
//META: purpose=An `EntryPointRequest` is "a function name plus a pipeline stage"; `-entry foo` on the command line selects the function named `foo` as the entry.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `-entry alpha` selects the function `alpha` as the
// entry point. The file declares two compute entries `alpha` and
// `beta`; with `-entry alpha` the emitted code must reference the
// constant written by `alpha` (the byte for 'A') and not the one
// written by `beta` (the byte for 'B').

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry alpha -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry alpha -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry alpha -stage compute

RWStructuredBuffer<int> outBuf;

void alpha()
{
    //HLSL: 65
    //GLSL: 65
    //WGSL: 65
    outBuf[0] = int('A');
}

void beta()
{
    // Should NOT appear in the emit when alpha is selected.
    outBuf[0] = int('B');
}
