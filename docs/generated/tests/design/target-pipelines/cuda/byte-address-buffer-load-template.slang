//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-c-cuda-legalization-lowering-phi-elimination
//META: doc_section_digest=4b70b0a669cc9b83124a252d2add0ed8fb3787692f5a88feaade1d3780fda053
//META: purpose=CUDA uses the default legalizeByteAddressBufferOps options so ByteAddressBuffer.Load<T> survives as a templated method call.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `legalizeByteAddressBufferOps` runs with default options for CUDA
// (CUDA is in the `default` arm of both byte-address-buffer
// switches). The result is a templated `(buf).Load<T>(offset)`
// call in the emitted CUDA C++ source.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

ByteAddressBuffer bab;
RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(8, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint v = bab.Load<uint>(tid.x * 4);
    outBuf[tid.x] = v;
}

// CHECK: Load<uint
