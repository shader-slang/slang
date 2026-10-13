//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/misc.md#capability-sets
//META: doc_section_digest=4df2fd6705515bd0c9e3c93ab41a3b3880a4851f2502821b5f9070869ee7b91d
//META: purpose=A `__requireCapability(...)` statement encodes its capability set as `capabilityConjunction` insts of integer atoms wrapped by an outer `capabilityDisjunction`.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a capability requirement is stored in the IR in
// disjunction-of-conjunctions normal form, with each conjunction operand an
// integer capability-atom value. The shader states one requirement and the
// CHECKs read the IR dump: a `capabilityConjunction` whose operands are
// `<n> : Int` atoms, and a `capabilityDisjunction` that ORs conjunction
// results together, which is only possible if the builder built the outer
// level from the per-atom-set conjunctions.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void main()
{
    __requireCapability(spirv_1_3);
    outputBuffer[0] = 1;
}

// IR-DAG: capabilityConjunction({{[0-9]+}} : Int
// IR-DAG: : CapabilitySet{{[ 	]*}}= capabilityDisjunction(%
