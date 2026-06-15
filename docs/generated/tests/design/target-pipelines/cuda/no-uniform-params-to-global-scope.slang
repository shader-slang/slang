//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:30:46+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-a-link-and-entry-point-prep
//META: doc_section_digest=f2c71c1e818a46614c1369ca340b627ac6a41920fa32fb97d4b7f11c78a611e0
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
