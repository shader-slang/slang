//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:27Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/llm-generated/ir-reference/generics-and-existentials.md#existential-destructuring
//META: doc_section_digest=02ff34102848a32784b08786fff8eb6145699715106f91aa1145c3476f2f96f7
//META: purpose=Dispatching through an interface-typed value produces extractExistentialValue(%i) reading the packed concrete-typed value.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `### Existential destructuring` lists `extractExistentialValue`
// as "Reads the packed concrete-typed value from an existential".
// The dump shows it appearing alongside `extractExistentialType`
// at any interface-dispatch site.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

interface IFoo { int run(int x); };
struct A : IFoo { int run(int x) { return x + 1; } };

uniform int inVal;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    A a;
    IFoo i = a;
    buf[0] = i.run(inVal);
}

// CHECK: extractExistentialValue(%i)
