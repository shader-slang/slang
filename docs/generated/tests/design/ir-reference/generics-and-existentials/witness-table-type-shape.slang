//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:08:34Z
//META: source_commit=8ac9e49a5b7020af6a8040544b9f76f0fbf40898
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#witness-tables-and-witness-facts
//META: doc_section_digest=8666fb5a8a3e866e04eed6573b9b735ade91d11b88b56b96aa10b6822be031b8
//META: purpose=A struct implementing an interface produces a witness_table whose operand 0 is the conforming concrete type, with the conformed interface carried by the result witness_table_t type.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the witness_table operand split: the inst's operand 0 is
// the conforming concrete type, while the conformed interface rides
// on the result WitnessTableType rather than on the inst itself. In
// the dump this reads `witness_table %N : witness_table_t(%IFoo)(%A)`
// — the type parenthesis holds the interface and the trailing operand
// list holds the concrete type, so the CHECK pins interface-in-type
// and concrete-type-as-operand in one line.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

interface IFoo { int run(int x); };
struct A : IFoo { int run(int x) { return x + 1; } };

uniform int inVal;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    A a;
    buf[0] = a.run(inVal);
}

// CHECK: witness_table %{{[0-9]+}}	: witness_table_t(%IFoo)(%A)
