//META: generated=true
//META: model=gpt-5.6-sol
//META: generated_at=2026-09-16T06:40:19Z
//META: source_commit=d3b56927b854c14fa1ac169b72da6e080ea80833
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#target-specific-lowering
//META: doc_section_digest=557b5025d6a79f84b833cef681013d91b8a66b0574857ec5bf62d3232bf54ca2
//META: purpose=Expansion: cuda-immutable-load wraps fields of a struct-valued `StructuredBuffer` element in `__ldg(&...)` on CUDA.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Struct-valued read-only buffer variation on cuda-immutable-load.
// CUDA expands the element load and wraps its field reads in
// `__ldg(&...)`.

//TEST:SIMPLE(filecheck=CUDA):-target cuda -stage compute -entry main

struct Settings { int factor; int bias; }

StructuredBuffer<Settings> cfg;
RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Settings settings = cfg[tid.x];
    buf[0] = settings.factor * int(tid.x) + settings.bias;
}

// CUDA: __ldg
