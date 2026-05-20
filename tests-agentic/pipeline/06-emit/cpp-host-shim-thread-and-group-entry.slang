//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/pipeline/06-emit.md#c
//META: doc_section_digest=98bae1408ef948818e6d18a391c8a3a89d2f13645057f93200d977f0f093501d
//META: purpose=Boundary: the C++ backend's entry-point shim emits both the per-thread (`_Thread`) and per-group (`_Group`) flavours so the host runtime can drive either dispatch level.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe of the C++ emit shape: the existing
// `cpp-prelude-and-export.slang` pins `SLANG_PRELUDE_EXPORT` and the
// `ComputeVaryingInput` parameter. This test pins the two shim
// flavours (`_Thread` and `_Group`) that the C++ backend emits to
// let the host runtime dispatch one thread at a time or one group
// at a time — a non-obvious detail of the C++ backend's contract
// with `source/slang-rt/`.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// CHECK: SLANG_PRELUDE_EXPORT
// CHECK: main_{{[0-9]+}}_Thread
// CHECK: SLANG_PRELUDE_EXPORT
// CHECK: main_{{[0-9]+}}_Group
