//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#shape-of-the-emitted-file
//META: doc_section_digest=cd40f48a68f8c4cc01781946c78837919b2702a1a94124525d0a69aadc68ec8c
//META: purpose=When emit finds an NVAPI-requiring instruction the front matter defines `SLANG_HLSL_ENABLE_NVAPI 1` and `NV_HITOBJECT_USE_MACRO_API 1` immediately before the `#pragma pack_matrix` line, which is what makes the prelude's guarded `nvHLSLExtns.h` include live.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the "live NVAPI" half of the front-matter rule. Every
// other NVAPI test in this bundle observes the inert case — the
// prelude's `#ifdef SLANG_HLSL_ENABLE_NVAPI` guard is present but
// the macro is never defined — which cannot distinguish "the
// front-matter branch works" from "the branch is dead". A
// `RWByteAddressBuffer.InterlockedAddF32` has no native HLSL
// intrinsic and is therefore emitted as the NVAPI call
// `NvInterlockedAddFp32`, so it is a source shape that forces the
// decoration. The CHECKs assert the two defines precede the pragma
// (order matters: they must be visible to the guarded include that
// follows) and that the NVAPI call itself reaches the body.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWByteAddressBuffer buf;
RWStructuredBuffer<float> outBuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float orig;
    buf.InterlockedAddF32(tid.x * 4, 1.0f, orig);
    outBuf[tid.x] = orig;
}

// CHECK: #define SLANG_HLSL_ENABLE_NVAPI 1
// CHECK-NEXT: #define NV_HITOBJECT_USE_MACRO_API 1
// CHECK-NEXT: #pragma pack_matrix(column_major)
// CHECK: #ifdef SLANG_HLSL_ENABLE_NVAPI
// CHECK-NEXT: #include "nvHLSLExtns.h"
// CHECK: NvInterlockedAddFp32
