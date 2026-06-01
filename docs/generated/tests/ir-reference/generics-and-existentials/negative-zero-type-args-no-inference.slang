//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:27Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#specialize
//META: doc_section_digest=4965484dbbae47fbcb43a84ac453b10df0094aeb1ec637158927580932cf4106
//META: purpose=A 0-arg `<>` call to a single-type-param generic with no argument-driven inference is rejected — the specialize operand list cannot be built without a type argument.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Negative on generic arity axis: 0 type args supplied for a
// generic that needs 1 type argument and provides no inference
// site. The doc's `### specialize` operand layout is `base,
// arg, ...` — without an `arg` the IR opcode is ill-defined and
// must be rejected before lowering.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv-asm -entry main -stage compute

interface IFoo { int run(int x); };

uniform RWStructuredBuffer<int> buf;

int doStuff<T : IFoo>() { return 0; }
/*CHECK:
    ^^^^^^^ see declaration of func doStuff<T> -> int
*/

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = doStuff<>();
//CHECK:              ^ could not specialize generic for arguments of type ()
}
