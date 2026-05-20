//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/llm-generated/ir-reference/resources-and-atomics.md#nonuniformresourceindex
//META: doc_section_digest=c8195cb5c6289df6fb0a637757c98be399b3a609f4400505427ffd0e8bf7939f
//META: purpose=`NonUniformResourceIndex(i)` is a no-op at the value level: the operand and result share the same type, so the surrounding indexing expression sees an integer of the same width.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says `nonUniformResourceIndex` carries a flag that
// backends translate into the `NonUniform` decoration. At the IR
// level, the result type matches the operand. Use a signed-index
// shape so this exercise differs from the unsigned form covered by
// `non-uniform-resource-index.slang`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<int> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int i = (int)tid.x;
    int nu = NonUniformResourceIndex(i);
    rwbuf[nu] = nu;
}

// IR-LABEL: func %main
// IR: nonUniformResourceIndex(%{{.*}})
