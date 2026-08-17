//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#existential-construction
//META: doc_section_digest=62ba2c4a9e08474dfeafec211473bf3bbcd1a4cfdd7108f3be2dc7058d0a51b2
//META: purpose=The core-module createDynamicObject builds an existential from a runtime type id, lowering to createExistentialObject(typeID, value).
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.
// Verifies the runtime-tagged construction form, which pairs a type id with a
// value instead of a static witness. `createDynamicObject<IFoo, A>` is the
// Slang surface declared with that intrinsic op; `-conformance` supplies the
// sequential id the target needs. The CHECK pins the resulting inst with the
// literal id first and the packed value second.
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute -conformance A:IFoo=0

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
    buf[0] = i.run(inVal);
}

// CHECK: let  %i : %IFoo = createExistentialObject(0 : UInt, %{{[0-9]+}})
