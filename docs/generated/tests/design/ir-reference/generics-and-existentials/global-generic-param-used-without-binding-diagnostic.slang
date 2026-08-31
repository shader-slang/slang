//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#generic-application
//META: doc_section_digest=e7e100a63c866d0df80fc71ebcb3d885933673afb6bac3a23c94ac816f7b50b1
//META: purpose=A module that uses a module-scope type_param without a concrete binding is rejected with E38207, reported once per source declaration rather than once per lowered global_generic_param.
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.
// Verifies both halves of the unbound-parameter rule. `type_param T : IFoo` is
// used in shader code with no binding supplied, so the reported E38207 pins the
// first half. The second half is what the test's exhaustive matching proves: a
// constrained `type_param` lowers to *two* module-scope parameters — a
// `Type`-typed one for `T` and a `witness_table_t(%IFoo)`-typed one for the
// conformance — yet the only annotations here are the short and long forms of a
// single report against the one source declaration. The run is exhaustive, so a
// second report for the synthesized witness parameter would fail the test.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv-asm -entry main -stage compute

interface IFoo { int run(int x); };

type_param T : IFoo;
//CHECK:   ^ error E38207
//CHECK:   ^ a global generic parameter ('type_param' / '__generic_value_param') cannot be used in shader code without a concrete binding

uniform int inVal;
uniform RWStructuredBuffer<int> buf;

int probe(T t, int x) { return t.run(x); }

[numthreads(1, 1, 1)]
void main()
{
    T t = {};
    buf[0] = probe(t, inVal);
}
