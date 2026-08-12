//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#existential-destructuring
//META: doc_section_digest=bfaba91cd7aecc07abcd6aed864d007e29538d6fe91106d3d1dd5e36f206a91a
//META: purpose=Dispatching through an interface-typed value produces extractExistentialValue(%i) reading the packed concrete-typed value.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.
// Verifies the value projection. Calling a method through an interface-typed
// local requires recovering the packed receiver first; the CHECK pins the
// single-operand `extractExistentialValue` naming that local.
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
