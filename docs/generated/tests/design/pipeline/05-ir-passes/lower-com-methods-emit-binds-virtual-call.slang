//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#specialization-and-generics
//META: doc_section_digest=4af16ce22c9a66bc00390c870ab8fa13bdbb68c7437a11e64d86f8f9c02cdbe4
//META: purpose=Lowering of dynamic dispatch resolves a known interface-typed value (concrete implementer) to a direct call in the emit.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The "Lower dynamic-dispatch insts" row of the specialization /
// generics table (`slang-ir-lower-dynamic-dispatch-insts.cpp`)
// resolves interface-typed values when the concrete implementer is
// known. The HLSL emit ends up calling the concrete `addOne` body —
// no `lookup_witness` or v-table remains.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main

interface IAdder { int add(int x); }

struct AddOne : IAdder { int add(int x) { return x + 1; } }

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    AddOne adder;
    buf[0] = adder.add(a);
}

// HLSL: void main
// HLSL-NOT: lookup_witness
