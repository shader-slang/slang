//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#branch--flatten--loopcontrol
//META: doc_section_digest=f2ba884d83c46721a4e1598e9622f13a2cab5e3cbdcc2f2993805ccbe9c8f247
//META: purpose=The loopControl unroll mode recorded from [unroll] flows through the IR unchanged and is re-emitted by the HLSL backend as the [unroll] attribute on the loop.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the unroll half of the pass-through claim: the compiler does not
// unroll the loop itself, it forwards the request. The HLSL CHECKs assert
// `[unroll]` is printed and a `for` statement is still there, which would
// not hold if the IR had consumed the hint by expanding the loop body. The
// GLSL and SPIR-V CHECKs assert the same surviving hint in those targets'
// own spellings - the `GL_EXT_control_flow_attributes` `[[unroll]]`
// attribute, and the `Unroll` loop-control mask on `OpLoopMerge`.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -O0 -entry main -stage compute

RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int sum = 0;

    [unroll]
    for (int i = 0; i < 8; i++)
    {
        sum += int(tid.x) + i;
    }

    outputBuffer[tid.x] = sum;
}

// HLSL: [unroll]
// HLSL: for

// GLSL: #extension GL_EXT_control_flow_attributes
// GLSL: {{\[\[}}unroll{{\]\]}}
// GLSL: for

// SPIRV: OpLoopMerge {{.*}} Unroll
