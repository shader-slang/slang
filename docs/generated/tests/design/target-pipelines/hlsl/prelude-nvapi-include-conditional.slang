//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:16:11+00:00
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=03d6fe7b8f4502160d35cd7e0f89a3145a7dfe7a6680c44cb1092265f9fe4de7
//META: purpose=HLSL emit wraps the nvHLSLExtns.h include behind #ifdef SLANG_HLSL_ENABLE_NVAPI so it is opt-in.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The HLSL prelude does NOT unconditionally include the NVAPI
// extension header; it gates the include on a user-defined macro
// so a downstream compile only pays for NVAPI if it has been
// enabled.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    outBuf[0] = 1;
}

// CHECK: #ifdef SLANG_HLSL_ENABLE_NVAPI
// CHECK: #include "nvHLSLExtns.h"
// CHECK: #endif
