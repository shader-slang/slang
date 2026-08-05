//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#metal
//META: doc_section_digest=2c63512e8c55eaa520cc1fe344ba5b272b854acc441029c722b381d592ccdc2a
//META: purpose=A `printf` in a Metal kernel records a logging requirement on the extension tracker, which the Metal backend satisfies by injecting `#include <metal_logging>` and lowering the call to `os_log_default.log(...)`.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the requirement-tracking half of the Metal backend: MSL has no
// `printf`, so the emitter must both request the logging extension and rewrite
// the call. CHECK asserts the extra system include appears in the preamble and
// that the call site became an `os_log_default.log` with the user's format
// string preserved. The dispatch id is used as the argument so the call cannot
// be folded away, and a buffer write keeps the kernel non-trivial. Contrast
// prefixes on HLSL and CUDA confirm those backends keep the plain `printf`.

//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    printf("v=%d\n", int(tid.x));
    outBuf[0] = int(tid.x);
}

// METAL-DAG: {{^}}#include <metal_logging>
// METAL-DAG: os_log_default.log("v=%d
// METAL-NOT: {{^}}    printf(

// HLSL: printf("v=%d
// CUDA: printf("v=%d
