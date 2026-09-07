//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#makediffpair
//META: doc_section_digest=9c6b7a39271eb0f667fd936efc160c0082114c1a92422a72493cec6e8b0fc6cb
//META: purpose=A module that constructs and projects a differential pair without mentioning fwd_diff or bwd_diff still runs the autodiff finalization passes, so no differentiation opcode reaches any target text.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the claim that `MakeDiffPair` is not autodiff-internal: plain user
// code gets one from `diffPair(x, dx)` even with no differentiation request in
// the module, and the finalization passes still run and rewrite it away. The
// shader builds a pair from a dispatch-derived value (so nothing is constant
// folded before the pair exists) and stores both halves. The CPU run asserts
// the primal and differential values survive the rewrite intact, and each
// text-emit target asserts that no `DiffPair` / `GetPrimal` / `GetDifferential`
// spelling appears anywhere in the generated source — the pair has been
// dissolved into ordinary values by emit time.

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

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    float a = float(tid.x) + 3.0;
    DifferentialPair<float> p = diffPair(a, a * 2.0);
    outputBuffer[0] = p.p;
    outputBuffer[1] = p.d;
}

// BUF: 3.0
// BUF-NEXT: 6.0

//HLSL-NOT: {{GetPrimal|GetDifferential|DiffPair}}
//HLSL: void computeMain(
//HLSL-NOT: {{GetPrimal|GetDifferential|DiffPair}}

//GLSL-NOT: {{GetPrimal|GetDifferential|DiffPair}}
//GLSL: void main()
//GLSL-NOT: {{GetPrimal|GetDifferential|DiffPair}}

//SPV-NOT: {{GetPrimal|GetDifferential|DiffPair}}
//SPV: OpEntryPoint GLCompute
//SPV-NOT: {{GetPrimal|GetDifferential|DiffPair}}

//METAL-NOT: {{GetPrimal|GetDifferential|DiffPair}}
//METAL: void computeMain(
//METAL-NOT: {{GetPrimal|GetDifferential|DiffPair}}

//WGSL-NOT: {{GetPrimal|GetDifferential|DiffPair}}
//WGSL: fn computeMain(
//WGSL-NOT: {{GetPrimal|GetDifferential|DiffPair}}

//CUDA-NOT: {{GetPrimal|GetDifferential|DiffPair}}
//CUDA: void computeMain()
//CUDA-NOT: {{GetPrimal|GetDifferential|DiffPair}}

//CPP-NOT: {{GetPrimal|GetDifferential|DiffPair}}
//CPP: computeMain
//CPP-NOT: {{GetPrimal|GetDifferential|DiffPair}}
