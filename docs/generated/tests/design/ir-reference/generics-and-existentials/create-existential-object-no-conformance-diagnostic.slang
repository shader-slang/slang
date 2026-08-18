//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#existential-construction
//META: doc_section_digest=62ba2c4a9e08474dfeafec211473bf3bbcd1a4cfdd7108f3be2dc7058d0a51b2
//META: purpose=A createExistentialObject whose interface has no conformance in the linkage stops code generation with E50100, "no type conformances found".
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.
// Verifies the linkage dependency of the runtime-tagged construction form. The
// existential built by `createDynamicObject` carries a runtime type id rather
// than a static witness, so the set of types it can hold has to come from the
// witness tables that survived linking. `A : IFoo` is referenced nowhere else,
// so nothing forces it into the link and that set is empty. This file is the
// negative counterpart of `create-existential-object-dynamic-object.slang`,
// which compiles the identical surface with `-conformance A:IFoo=0` supplied;
// the only difference is the missing option, so the E50100 the caret pins is
// attributable to the empty conformance set and nothing else.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv-asm -entry main -stage compute

interface IFoo { int run(int x); };
struct A : IFoo { int v; int run(int x) { return v + x; } };

uniform int inVal;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    A a;
    a.v = inVal;
    IFoo i = createDynamicObject<IFoo, A>(0, a);
//CHECK:                                 ^ error E50100
//CHECK:                                 ^ No type conformances are found for interface 'IFoo'.
    buf[0] = i.run(inVal);
}
