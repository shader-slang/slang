//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#specialization-and-generics
//META: doc_section_digest=80bc5973053356e62dbd6f6ded27b7e93976729f33655c137440d5c954689e89
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
