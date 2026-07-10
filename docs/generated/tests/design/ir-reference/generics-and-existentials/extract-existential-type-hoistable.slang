//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:08:34Z
//META: source_commit=8ac9e49a5b7020af6a8040544b9f76f0fbf40898
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#extractexistentialvalue-extractexistentialtype-extractexistentialwitnesstable
//META: doc_section_digest=16045cc2106f4346cc8dfb52e07857b9a6e4297569798ffd3bcd3f5258a250c1
//META: purpose=Two dispatches through the same existential share a single hoistable extractExistentialType inst, while each dispatch keeps its own non-hoistable extractExistentialValue.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the hoistability asymmetry of the projection trio: the type
// projection's result is determined purely by its existential operand,
// so two dispatch sites on the same existential resolve to one
// extractExistentialType inst, whereas the value projection is created
// per site. The CHECK captures the single %T definition and then
// requires *two* separate extractExistentialValue lines whose result
// type is that same captured %T — only a deduplicated type projection
// can be the result type of both value projections.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

interface IFoo { int run(int x); int walk(int x); };
struct A : IFoo
{
    int run(int x) { return x + 1; }
    int walk(int x) { return x + 2; }
};

uniform int inVal;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    A a;
    IFoo i = a;
    buf[0] = i.run(inVal);
    buf[1] = i.walk(inVal);
}

// CHECK: let  [[T:%[0-9]+]] : Type = extractExistentialType(%i)
// CHECK: : [[T]] = extractExistentialValue(%i)
// CHECK: : [[T]] = extractExistentialValue(%i)
