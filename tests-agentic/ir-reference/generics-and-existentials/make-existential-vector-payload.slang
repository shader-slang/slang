//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:27Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/llm-generated/ir-reference/generics-and-existentials.md#makeexistential
//META: doc_section_digest=3a7462a86fc4c15dc7bc9c9156d37eabae3d8e2c13a7f3bce50ff7cff1572439
//META: purpose=A makeExistential whose concrete-type payload contains a vector field still produces a let %i : %IFoo with the two-operand value/witness shape.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary on makeExistential payload shape: concrete type
// holding a vector field. `### makeExistential` says the result
// is "a single existential value of type `I`" regardless of the
// shape of the packed value — the `let %i : %IFoo` shape and
// two-operand form are invariant across payload kinds.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

interface IFoo { int run(int x); };

struct V : IFoo
{
    float3 v;
    int run(int x) { return int(v.x) + x; }
};

uniform float3 inVec;
uniform int inVal;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    V s;
    s.v = inVec;
    IFoo i = s;
    buf[0] = i.run(inVal);
}

// CHECK: let  %i	: %IFoo	= makeExistential(%{{[A-Za-z0-9_]+}}, %{{[0-9]+}})
