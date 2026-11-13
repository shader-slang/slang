//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#specialize
//META: doc_section_digest=cb05174ca18de9aec7cd0f1b2d0a9f691c8420254c205a2b019628a2408e35e5
//META: purpose=A 0-arg `<>` call to a single-type-param generic with no argument-driven inference is rejected.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.
// Boundary-negative on generic arity zero: with an empty argument list and no
// call arguments to infer from, the argument tail a `specialize` needs cannot
// be built. The CHECKs pin the could-not-specialize diagnostic at the empty
// bracket pair and the note pointing at the declaration.
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
