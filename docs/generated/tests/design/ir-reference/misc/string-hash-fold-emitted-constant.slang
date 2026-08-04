//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:12:20Z
//META: source_commit=5f0e613fdd3c2890bb087d9ff76154e2605a14d2
//META: doc_ref=docs/generated/design/ir-reference/misc.md#getstringhash
//META: doc_section_digest=b534354f891756509322eeb68d274d9dc67208cbe58e97b555830eba07b9e425
//META: purpose=The emitters fold `getStringHash` to its constant hash value, so the emitted target code does not carry the string literal's bytes.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that getStringHash of a literal is folded to its constant
// hash before target emission, so the literal's bytes never reach the
// backend text. The hash result escapes through a buffer store; on
// every text-emit target the CHECK-NOT pairs assert the unique marker
// substring is absent both before and after the entry point, which can
// only hold if the emitter replaced the inst with the folded constant.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute

RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = getStringHash("fnv1a-fold-probe-9137");
}

//HLSL-NOT: fnv1a-fold-probe-9137
//HLSL: computeMain
//HLSL-NOT: fnv1a-fold-probe-9137

//GLSL-NOT: fnv1a-fold-probe-9137
//GLSL: void main
//GLSL-NOT: fnv1a-fold-probe-9137

//SPV-NOT: fnv1a-fold-probe-9137
//SPV: OpEntryPoint
//SPV-NOT: fnv1a-fold-probe-9137

//METAL-NOT: fnv1a-fold-probe-9137
//METAL: computeMain
//METAL-NOT: fnv1a-fold-probe-9137

//WGSL-NOT: fnv1a-fold-probe-9137
//WGSL: computeMain
//WGSL-NOT: fnv1a-fold-probe-9137

//CUDA-NOT: fnv1a-fold-probe-9137
//CUDA: computeMain
//CUDA-NOT: fnv1a-fold-probe-9137

//CPP-NOT: fnv1a-fold-probe-9137
//CPP: computeMain
//CPP-NOT: fnv1a-fold-probe-9137
