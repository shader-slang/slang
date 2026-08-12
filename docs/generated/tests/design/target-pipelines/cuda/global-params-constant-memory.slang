//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-a-link-and-entry-point-prep
//META: doc_section_digest=f2b0f55be3f7599d8bcbf2797ba30a8b014ef4b949f1acb1eef1c1fcd4a0be49
//META: purpose=Global uniform parameters are gathered into a GlobalParams_0 struct bound to __constant__ memory because moveEntryPointUniformParamsToGlobalScope is skipped for CUDA.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `collectGlobalUniformParameters` (always-on in Phase A, plus the CUDA
// `collectOptiXEntryPointUniformParams`-instead-of-`collectEntryPointUniformParams`
// substitution) packs module-scope uniforms into a single struct.
// The CUDA emitter places that struct in `__constant__` memory.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

RWStructuredBuffer<int> outBuf;
StructuredBuffer<float> roBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(roBuf[tid.x]);
}

// CHECK: struct GlobalParams_0
// CHECK: extern "C" __constant__ GlobalParams_0 SLANG_globalParams
