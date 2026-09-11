//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#glsl-preprocessor--layout--format
//META: doc_section_digest=3e91a35f798312a950be8e05675e12a27f7b98c1c164b5734412c26b64bea700
//META: purpose=The `globallycoherent` memory qualifier reaches both the HLSL and the GLSL back end.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the memory qualifier survives to codegen on each back end under
// that back end's own spelling: HLSL keeps `globallycoherent`, while GLSL spells
// device-scope coherence as the `coherent` qualifier on the buffer block. Two
// directives with separate prefixes assert the respective emitted token, so a
// regression on either target fails this file.

// SPIR-V records the same device-scope coherence as a `Coherent` decoration on
// the buffer variable, so the SPIRV prefix completes the Khronos half of the
// claim.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute

globallycoherent RWStructuredBuffer<int> outBuf;

[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

//HLSL: globallycoherent
//GLSL: coherent
//SPIRV: OpDecorate %outBuf Coherent
