//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:30:46+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-c-cuda-legalization-lowering-phi-elimination
//META: doc_section_digest=2314d32203f54be9e61ab85654673e194c467242384d7feb583ce0587536f9da
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
