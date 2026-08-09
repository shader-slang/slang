//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/serialization.md#source-location-serialization
//META: doc_section_digest=0b9df2a75e79b7e8b6026192b354673f12b076885b33df7d9612c34bd2b3f441
//META: purpose=With debug info requested, the per-file path and the line numbers reached by source locations both reach SPIR-V, as an OpString naming this file plus DebugSource and DebugLine instructions.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the two halves of what a serialized source location keeps
// per contributing file - the file's path, and line records only for
// the lines locations actually reach - are separately observable when
// -g2 turns on debug info. The CHECKs require an OpString holding this
// file's path, a DebugSource that binds that string to the module, and
// at least one DebugLine carrying line numbers. A location reduced to a
// bare integer would produce the DebugLine entries with no path to hang
// them off, so requiring all three together is the point.
//
// This claim is SPIR-V-only by construction: OpString, DebugSource and
// DebugLine are SPIR-V instructions with no counterpart in any text
// emitter, so there is no emission fan-out to write here. Under -g2 the
// HLSL and Metal emitters render the same retained locations as
// `#line <n> "<path>"`, GLSL as `#line <n> <fileNumber>` with no path,
// and WGSL emits nothing - separate claims that the sibling
// source-loc-line-directives-multi-target.slang and
// source-loc-line-directive-mode-variants.slang tests already assert.
// The four absent targets are therefore recorded as
// unsupported-on-target in the bundle README.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute -g2

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = 99;
}

// CHECK-DAG: OpString "{{.*source-loc-spirv-debug-info\.slang}}"
// CHECK-DAG: DebugSource
// CHECK-DAG: DebugLine
