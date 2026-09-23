//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#downstream-nvrtc
//META: doc_section_digest=5fd8b654610eb53a347bc6fa7bbe231ed19030077a9448cce02372f505e1eca7
//META: purpose=-target ptx triggers the downstream nvrtc invocation that translates the CUDA C++ text into PTX assembly with a .version header and a .visible .entry kernel symbol.
//META: intent=expansion
//META: pipeline_stage=emit
//META: requires-tool=nvrtc
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
