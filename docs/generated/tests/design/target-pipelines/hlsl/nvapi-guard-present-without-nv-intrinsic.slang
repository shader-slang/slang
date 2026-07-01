//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:31:07+00:00
//META: source_commit=019ec0e5ddac1c14fbad141a3f03d50cf8a5e022
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=03d6fe7b8f4502160d35cd7e0f89a3145a7dfe7a6680c44cb1092265f9fe4de7
//META: purpose=Boundary: even when the shader uses NO NV intrinsic, the prelude still emits the SLANG_HLSL_ENABLE_NVAPI guard for nvHLSLExtns.h.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: NV-extension conditional emit — the include
// guard is part of the unconditional prelude. The shader body
// here is the simplest possible — no wave/NV intrinsics — yet
// the guard still appears.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    outBuf[0] = 42;
}

// CHECK: #ifdef SLANG_HLSL_ENABLE_NVAPI
// CHECK: #include "nvHLSLExtns.h"
// CHECK: #endif
