//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#makeexistential
//META: doc_section_digest=12fc3bdfdfd39992f73247a51ecf438875c6398134a12dce64d1e764d8adbf0f
//META: purpose=A makeExistential whose concrete-type payload contains a vector field still produces a let %i : %IFoo with the two-operand value/witness shape.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.
// Boundary on payload *shape*: a vector-typed field is a legalization-sensitive
// member on several targets, but existential packing happens before any of
// that. The CHECK asserts the same two-operand shape and `%IFoo` result type.
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o - -entry main -stage compute

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

// CHECK: let  %i : %IFoo = makeExistential(%{{[A-Za-z0-9_]+}}, %{{[0-9]+}})
