//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/llm-generated/ir-reference/resources-and-atomics.md#nonuniformresourceindex
//META: doc_section_digest=c8195cb5c6289df6fb0a637757c98be399b3a609f4400505427ffd0e8bf7939f
//META: purpose=`NonUniformResourceIndex(i)` surfaces as `nonUniformResourceIndex(%i)` — a no-op at the value level whose operand and result share the same type.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says `nonUniformResourceIndex(index)` is the IR encoding
// of HLSL's `NonUniformResourceIndex` intrinsic; the operand and
// result share the same type so that backends can later attach the
// `NonUniform` decoration.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<uint> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint nu = NonUniformResourceIndex(tid.x);
    rwbuf[nu] = tid.x;
}

// IR-LABEL: func %main
// IR: nonUniformResourceIndex(%{{.*}})
