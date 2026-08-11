//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#preludes
//META: doc_section_digest=2ee29be32e5ff124c3382011f81838f97168cb1d820c38d6cd432c675f3663c7
//META: purpose=GLSL, Metal, WGSL and SPIR-V output carries no prelude/ header reference; their builtin vocabulary comes from the back end or the downstream toolchain.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the prelude mechanism is specific to the C-family text
// targets. The identical shader that makes the C++ and CUDA back ends
// emit a slang-*-prelude.h reference is compiled for the four back
// ends the doc carves out; each prefix uses a -NOT over the module
// header region followed by a positive anchor deep in the body, so a
// prelude line appearing anywhere before the entry point would fail.
// Metal's metal_stdlib include is a downstream-toolchain header, not a
// prelude/ artefact, so it is not what the -NOT looks for.

//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// GLSL-NOT: prelude.h
// GLSL: void main()
// SPIRV-NOT: prelude.h
// SPIRV: OpEntryPoint GLCompute
// METAL-NOT: prelude.h
// METAL: kernel{{.*}}void computeMain
// WGSL-NOT: prelude.h
// WGSL: fn computeMain
