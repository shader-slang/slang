//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-a-link-and-entry-point-prep
//META: doc_section_digest=702ad77281db592f882065e33506b581a86d6d52898e040f800018dc367fd9bf
//META: purpose=moveEntryPointUniformParamsToGlobalScope is skipped for CUDA so an entry-point uniform parameter is read through the GlobalParams_0 constant struct rather than a kernel argument.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// CUDA is in the explicit skip list (line ~1126) for
// `moveEntryPointUniformParamsToGlobalScope`. Globals collected into
// `GlobalParams_0` go to `__constant__` memory and the kernel reads
// them via `globalParams_0` rather than receiving them as kernel
// arguments. The kernel signature is therefore `void main_0()`,
// not `void main_0(GlobalParams ...)`.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

cbuffer Cb { int scale; }
RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(8, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = scale * int(tid.x);
}

// CHECK: globalParams_{{[0-9]+}}
// CHECK: extern "C" __global__ void main_{{[0-9]+}}()
