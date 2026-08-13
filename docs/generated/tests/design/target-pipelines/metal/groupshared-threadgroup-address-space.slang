//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#specializeaddressspaceformetal
//META: doc_section_digest=1b089b6b695f999790efc3d28d9d88f3dad39630f0ab1202806c336fe9cfe4fd
//META: purpose=`specializeAddressSpaceForMetal` annotates a `groupshared` array so it emits as `threadgroup array<T, int(N)>` inside the kernel.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that Metal's explicit address spaces are assigned in
// `linkAndOptimizeIR` rather than deferred to the emitter: a `groupshared`
// declaration has to reach emit already carrying the `threadgroup` space,
// or MSL would reject the declaration. The array is written and then read
// so it cannot be eliminated; CHECK asserts the emitted declaration is
// `threadgroup array<int, int(64)>`.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

groupshared int s_data[64];
RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(64, 1, 1)]
void main(uint tid : SV_GroupIndex)
{
    s_data[tid] = int(tid);
    outBuf[tid] = s_data[tid];
}

// CHECK: threadgroup array<int, int(64)>
