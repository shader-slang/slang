//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#getsequentialid-and-the-rtti-opcodes
//META: doc_section_digest=c6932c779e6b7ce0b79c57daf536a4c05b33003e381d410c327ed5ffe0c10c37
//META: purpose=An optional-constraint check lowers to GetSequentialID on the constraint's witness table, yielding a UInt compared against the sentinel none id.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.
// Verifies how an optional conformance is tested at run time. `where optional
// T : IFoo` gives the generic a witness-table parameter that may be the "none"
// witness, and `T is IFoo` asks whether it is. The CHECK-DAGs assert the
// witness-table-typed generic parameter, the `GetSequentialID` whose result is
// a `UInt`, and the inequality against the all-ones sentinel that encodes
// "no conformance".
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

interface IFoo { int run(int x); };
struct A : IFoo { int run(int x) { return x + 1; } };

uniform int inVal;
uniform RWStructuredBuffer<int> buf;

int use<T>(T t, int x) where optional T : IFoo
{
    if (T is IFoo)
    {
        return t.run(x);
    }
    return x;
}

[numthreads(1, 1, 1)]
void main()
{
    A a;
    buf[0] = use<A>(a, inVal);
}

// CHECK-DAG: param %{{[0-9]+}} : witness_table_t(%IFoo))
// CHECK-DAG: let  %{{[0-9]+}} : UInt = GetSequentialID(%{{[0-9]+}})
// CHECK-DAG: cmpNE(%{{[0-9]+}}, 4294967295 : UInt)
