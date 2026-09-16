//META: generated=true
//META: model=gpt-5.6-sol
//META: generated_at=2026-09-16T07:46:01Z
//META: source_commit=d3b56927b854c14fa1ac169b72da6e080ea80833
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#how-target-choice-affects-ir
//META: doc_section_digest=1103b6c6793a74ba9a2a797712002157cc506284940af6ab698172cadfb6ec8a
//META: purpose=The CUDA-specific immutable-load lowering pass rewrites read-only `StructuredBuffer` loads as `__ldg(...)` calls on CUDA only, while the other text targets use their ordinary buffer-load forms.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Per the doc's "How target choice affects IR" list, CUDA carries a
// dedicated pass `slang-ir-cuda-immutable-load.cpp`. Its observable
// effect is that a read-only `StructuredBuffer` load lowers to a
// `__ldg(&...)` intrinsic on CUDA. The same load on every other
// text-emit target uses that target's ordinary buffer-load form.

//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute

StructuredBuffer<int> inBuf;
RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = inBuf[tid.x];
}

// CUDA: __ldg(
// HLSL-NOT: __ldg
// HLSL: inBuf_{{[0-9]+}}.Load(
// HLSL-NOT: __ldg

// Every shader text target other than CUDA uses its ordinary buffer
// representation. Each positive anchor proves that real emitted code
// was inspected, while the surrounding `-NOT` checks prove that no
// CUDA `__ldg` intrinsic appears anywhere in that target's output.

// GLSL-NOT: __ldg
// GLSL: = inBuf_{{[0-9]+}}._data[
// GLSL-NOT: __ldg
// SPIRV-NOT: __ldg
// SPIRV: OpLoad
// SPIRV-NOT: __ldg
// METAL-NOT: __ldg
// METAL: = inBuf_{{[0-9]+}}[
// METAL-NOT: __ldg
// WGSL-NOT: __ldg
// WGSL: = inBuf_{{[0-9]+}}[
// WGSL-NOT: __ldg
// CPP-NOT: __ldg
// CPP: inBuf_{{[0-9]+}}.Load(
// CPP-NOT: __ldg
