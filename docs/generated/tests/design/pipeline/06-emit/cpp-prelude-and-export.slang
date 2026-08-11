//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#c
//META: doc_section_digest=3b18f76865892822e5e99d93d6ae89f550064767414bd96151ede2d71030fdc5
//META: purpose=The C++ backend emits C++ source paired with `slang-cpp-prelude.h` and exports the compute entry point through `SLANG_PRELUDE_EXPORT` shims.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the C++ backend emits code that depends on the shipped C++ prelude
// and that the kernel is reachable from a host. CHECK asserts the prelude
// include (matched by filename because the emitted path is absolute and
// machine-specific), the prelude-provided `RWStructuredBuffer` template used
// for the global-parameter field, and the exported per-thread and per-group
// dispatch shims a host runtime calls.

//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(4, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = float(tid.x) * 0.5f;
}

// CPP: slang-cpp-prelude.h
// CPP-DAG: RWStructuredBuffer<float> outBuf_{{[0-9]+}};
// CPP-DAG: SLANG_PRELUDE_EXPORT
// CPP-DAG: void main_{{[0-9]+}}_Thread(ComputeThreadVaryingInput*
// CPP-DAG: void main_{{[0-9]+}}_Group(ComputeVaryingInput*
