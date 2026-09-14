//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-d-cuda-emit-and-downstream-tools
//META: doc_section_digest=6125db2deed527f9850790324fd3a09c7d0cd76e0bbeee263f331f56a3527a81
//META: purpose=The shared SourceWriter emits #line N "<file>" directives in the CUDA C++ text so nvrtc can map errors back to Slang source.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// CUDASourceEmitter inherits from the C-like emitter family that
// shares the `SourceWriter`. The default behavior emits `#line N
// "..."` directives in the output so the downstream compiler can
// report errors against the original source.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// CHECK: #line {{[0-9]+}} {{.*}}
