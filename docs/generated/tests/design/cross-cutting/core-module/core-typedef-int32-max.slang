//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#what-the-core-module-provides
//META: doc_section_digest=8a82ae360bcd088373acbb80719dc095678434371e751937b8feacab2eae3d90
//META: purpose=Boundary: `int32_t` accepts MAX literal 2147483647; lowers per target without overflow diagnostic.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: integer MAX — `int32_t` at 2147483647.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute

RWStructuredBuffer<int32_t> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int32_t maxInt = 2147483647;
    outBuf[tid.x] = maxInt - int32_t(tid.x);
}

// HLSL: 2147483647
