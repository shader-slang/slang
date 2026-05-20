//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=019ec0e5ddac1c14fbad141a3f03d50cf8a5e022
//META: doc_ref=docs/llm-generated/ir-reference/resources-and-atomics.md#nonuniformresourceindex
//META: doc_section_digest=c8195cb5c6289df6fb0a637757c98be399b3a609f4400505427ffd0e8bf7939f
//META: purpose=`NonUniformResourceIndex(0xFFFFFFFFu)` (UINT_MAX literal) still surfaces as `nonUniformResourceIndex(%i)` with the same uint type.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe: largest representable uint as the operand.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<uint> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint nu = NonUniformResourceIndex(0xFFFFFFFFu);
    rwbuf[nu & 0xff] = tid.x;
}

// IR-LABEL: func %main
// IR: nonUniformResourceIndex({{.*}})
