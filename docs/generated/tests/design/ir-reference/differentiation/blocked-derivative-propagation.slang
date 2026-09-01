//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#detachderivative
//META: doc_section_digest=01132bdcdbc1c50944722b06abe5d963b03cd1175d44d81a16fc40d947d9c916
//META: purpose=detachDerivative blocks derivative propagation through its operand and is deleted once differentiation is done, so no detach spelling survives into any target text.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies both halves of the claim end to end. The differentiable function
// computes `x * detach(x)`, so an unblocked derivative would be `2 * x * dx`
// while a blocked one is `x * dx`; the CPU run feeds x = 3 with dx = 1 and the
// buffer check requires 9 for the primal and 3 — not 6 — for the differential,
// which only holds if propagation really stopped at the detached operand. Each
// text-emit target then asserts that the emitted source contains the lowered
// pair struct but no `detach` spelling at all, since the marker is removed once
// the autodiff passes finish.

//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF):-cpu -output-using-type
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[Differentiable]
float f(float x)
{
    return x * detach(x);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    float a = float(tid.x) + 3.0;
    DifferentialPair<float> r = __fwd_diff(f)(diffPair(a, 1.0));
    outputBuffer[0] = r.p;
    outputBuffer[1] = r.d;
}

// BUF: 9.0
// BUF-NEXT: 3.0

//HLSL-NOT: detach
//HLSL: struct DiffPair_float
//HLSL-NOT: detach

//GLSL-NOT: detach
//GLSL: struct DiffPair_float
//GLSL-NOT: detach

//SPV-NOT: detach
//SPV: OpEntryPoint GLCompute
//SPV-NOT: detach

//METAL-NOT: detach
//METAL: struct DiffPair_float
//METAL-NOT: detach

//WGSL-NOT: detach
//WGSL: struct DiffPair_float
//WGSL-NOT: detach

//CUDA-NOT: detach
//CUDA: struct DiffPair_float
//CUDA-NOT: detach

//CPP-NOT: detach
//CPP: struct DiffPair_float
//CPP-NOT: detach
