//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/llm-generated/target-pipelines/cuda.md#downstream-nvrtc
//META: doc_section_digest=8526e0a7ba1a3f8cb12721f5aa8d1eab984f2e7568e1a48784b7f3070691fb26
//META: purpose=-target ptx triggers the downstream nvrtc invocation that translates the CUDA C++ text into PTX assembly with a .version header and a .visible .entry kernel symbol.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Per `## Downstream nvrtc`: `-target ptx` invokes nvrtc on the
// emitted CUDA C++ text to produce PTX. The PTX text artifact
// is fingerprinted by the `.version`/`.target` header and a
// `.visible .entry <name>` declaration for the compute kernel.

//TEST:SIMPLE(filecheck=CHECK):-target ptx -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// CHECK: .version
// CHECK: .visible .entry
// CHECK-SAME: main
