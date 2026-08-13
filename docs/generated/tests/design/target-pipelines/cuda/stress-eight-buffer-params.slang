//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-a-link-and-entry-point-prep
//META: doc_section_digest=f2b0f55be3f7599d8bcbf2797ba30a8b014ef4b949f1acb1eef1c1fcd4a0be49
//META: purpose=Stress: eight RWStructuredBuffer parameters all packed into GlobalParams_0 on CUDA emit because moveEntryPointUniformParamsToGlobalScope is skipped.
//META: intent=stress
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress axis: many global resource parameters. The
// `collectGlobalUniformParameters` pass packs all of them into a
// single `GlobalParams_0` struct rather than emitting them as
// kernel arguments.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

RWStructuredBuffer<int> a;
RWStructuredBuffer<int> b;
RWStructuredBuffer<int> c;
RWStructuredBuffer<int> d;
RWStructuredBuffer<int> e;
RWStructuredBuffer<int> f;
RWStructuredBuffer<int> g;
RWStructuredBuffer<int> h;
RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = a[tid.x] + b[tid.x] + c[tid.x] + d[tid.x]
                  + e[tid.x] + f[tid.x] + g[tid.x] + h[tid.x];
}

// CHECK: struct GlobalParams_0
// CHECK: RWStructuredBuffer<int> a_
// CHECK: RWStructuredBuffer<int> h_
// CHECK: extern "C" __global__ void main_{{[0-9]+}}()
