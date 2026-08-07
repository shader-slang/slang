//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#extractexistentialvalue--extractexistentialtype--extractexistentialwitnesstable
//META: doc_section_digest=2831394637fc16470e6426b0ef26aa0c691647c1fa7ec6c0ecb2e6e5d269f621
//META: purpose=Two dispatches through the same existential share a single hoistable extractExistentialType inst, while each dispatch keeps its own non-hoistable extractExistentialValue.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.
// Boundary that separates the hoistable projections from the non-hoistable
// one. Two method calls on the same existential are made; the CHECK captures
// the type projection once and then asserts that *both* value projections are
// typed by that same captured value — one shared type projection, two distinct
// value projections.
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
