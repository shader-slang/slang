//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:31:34+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/llm-generated/architecture/module-map.md#prelude-per-target-prelude-headers
//META: doc_section_digest=d9886192d9b600fb4caa92a3d5508dd5b3ad9e7014a6c043db7d23ea0f36e19c
//META: purpose=The `prelude/` directory ships one prelude header per textual target; CUDA emit literally includes `slang-cuda-prelude.h`.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the per-target prelude inventory claim: CUDA emit
// contains an `#include` of `slang-cuda-prelude.h`, naming the
// exact file the module map cites. If the prelude file were renamed
// or removed, the emit would not contain this string.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    //CHECK: slang-cuda-prelude.h
    outBuf[tid.x] = 1;
}
