//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#generics-and-existentials
//META: doc_section_digest=f9fbc5ab50420dd2b267ddb86a5eff193a88f8bd53dbaeb86a9ca9fdcaf05b23
//META: purpose=In a conformance to a generic interface, an entry satisfied by a member of the conforming type stays flat while an entry inherited from the interface's own default implementation gains an extra base-specialization layer — `specialize(specialize(%impl, Int), %S, %table)`.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a requirement witness taken out of the AST requirement
// dictionary is specialized through the *base* (interface) side of the
// conformance before being lowered, so an entry copied from a generic
// interface carries the conforming type's substitutions instead of the
// interface's own parameters. `S : IFoo<int>` gives the two shapes side by
// side: `zero` is implemented by `S` itself, mentions no interface parameter,
// and lowers flat; `twice` comes from the interface's default body, so its
// entry has an inner `specialize(..., Int)` binding the interface's `T` before
// the outer `specialize` supplies `%S` and its table. Asserting the two rows
// in the same table is what makes the extra layer attributable to the base
// specialization rather than to generic use in general.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

interface IFoo<T> { T zero(); T twice() { return zero(); } }
struct S : IFoo<int> { int zero() { return 0; } }

RWStructuredBuffer<int> outputBuffer;

int use<U : IFoo<int>>(U u) { return u.twice(); }

[numthreads(1, 1, 1)]
void main()
{
    S s;
    outputBuffer[0] = use(s);
}

// The table itself is over the specialized interface.
// CHECK: witness_table %[[TBL:[A-Za-z0-9_]+]]{{.*}}witness_table_t(specialize(%{{[A-Za-z0-9_]+}}, Int))(%S)
// Satisfied by a member of S: flat, no specialize layer.
// CHECK: witness_table_entry(%{{[A-Za-z0-9_]*}}zero,%{{[A-Za-z0-9_]*}}zero)
// Inherited default implementation: base specialization nests inside.
// CHECK: witness_table_entry(%{{[A-Za-z0-9_]*}}twice,specialize(specialize(%{{[A-Za-z0-9_]+}}, Int), %S, %[[TBL]]))
