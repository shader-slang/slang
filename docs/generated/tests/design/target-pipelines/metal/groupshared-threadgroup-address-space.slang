//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:50:00Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#specializeaddressspaceformetal
//META: doc_section_digest=80c941477d24c10fff0c5bef4c1b8c2e3af5b3a9a05d65d62133e771249cc15e
//META: purpose=`groupshared T[N]` emits with Metal's `threadgroup` address space as `threadgroup array<T, int(N)>` thanks to specializeAddressSpaceForMetal.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `specializeAddressSpaceForMetal` annotates the IR pointer
// types feeding the emitter so a `groupshared` array becomes a
// `threadgroup array<T, int(N)>` declaration inside the kernel.

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
