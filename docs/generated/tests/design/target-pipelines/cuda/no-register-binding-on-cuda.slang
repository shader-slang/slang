//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#cuda-specific-runtime-predicates
//META: doc_section_digest=f602d16d3b6f0697faae50ed1f67311176b68fdea089e235484c538cf18386b4
//META: purpose=CUDA is not isD3DTarget and is non-Khronos so HLSL register(uN) bindings do not appear in CUDA emit; resources are reached through GlobalParams_0.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The `isCUDATarget(targetRequest)` predicate selects a code path
// that emits resources as struct fields of `GlobalParams_0` in
// `__constant__` memory rather than as HLSL-style
// `register(uN)`-annotated globals.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

RWStructuredBuffer<int> outBuf;
ByteAddressBuffer bab;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(bab.Load<uint>(tid.x * 4));
}

// CHECK: globalParams_{{[0-9]+}}
// CHECK-NOT: register(u
// CHECK-NOT: register(t
