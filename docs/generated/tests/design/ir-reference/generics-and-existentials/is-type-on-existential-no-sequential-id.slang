//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#getsequentialid-and-the-rtti-opcodes
//META: doc_section_digest=c6932c779e6b7ce0b79c57daf536a4c05b33003e381d410c327ed5ffe0c10c37
//META: purpose=A run-time type test on an ordinary existential emits IsType and involves no GetSequentialID, unlike the optional-constraint form.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.
// Verifies the carve-out that separates the two arms of the `is` lowering. The
// sibling test `get-sequential-id-optional-constraint.slang` pins the arm that
// does produce a `GetSequentialID`: a `where optional T : IFoo` constraint
// tested with `T is IFoo`. Here the test subject is instead a plain existential
// — `i` has interface type `IFoo` and is tested against the concrete `A` — so
// the other arm runs. The CHECK pins the `IsType` inside `func %main` with `%A`
// as the tested type, and the two CHECK-NOTs bracket it so that no
// `GetSequentialID` may appear anywhere in the dump, before or after.
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

interface IFoo { int run(int x); };
struct A : IFoo { int v; int run(int x) { return v + x; } };

uniform int inVal;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    A a;
    a.v = inVal;
    IFoo i = a;
    buf[0] = (i is A) ? i.run(inVal) : 0;
}

// CHECK-NOT: GetSequentialID
// CHECK: func %main
// CHECK-NOT: GetSequentialID
// CHECK: = IsType(%{{[A-Za-z0-9_]+}}, %{{[A-Za-z0-9_]+}}, %A,
// CHECK-NOT: GetSequentialID
