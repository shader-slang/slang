//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#generic-interface-requirements
//META: doc_section_digest=2baff5b5f1d715938cf9566d6f570582a342e5719c0e3486b541366359660b9d
//META: purpose=A generic interface method's satisfying implementation is stored in the conforming type's witness table as a requirement-local `IRGeneric`, not as a flat function value.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that an interface requirement which is itself generic keeps its own
// generic parameters in the witness-table entry. `IOps.apply<T>` is generic, so
// `S`'s implementation cannot be lowered as a plain function: lowering opens a
// nested environment, emits the requirement's own generic parameters, and
// stores the resulting generic as the entry value. The CHECKs capture the id of
// the generic emitted for `S.apply` and require the same id to appear as the
// value operand of the `witness_table_entry` row in `S`'s table.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

interface IOps
{
    T apply<T : IArithmetic>(T v);
}

struct S : IOps
{
    T apply<T : IArithmetic>(T v) { return v + v; }
}

uniform int a;
uniform RWStructuredBuffer<int> buf;

int use<U : IOps>(U u, int x)
{
    return u.apply<int>(x);
}

[numthreads(1, 1, 1)]
void main()
{
    S s;
    buf[0] = use(s, a);
}

// CHECK: [export("{{.*}}1S5apply{{.*}}")]
// CHECK-NEXT: generic [[G:%[A-Za-z0-9_]+]] :
// CHECK: [nameHint("S.apply")]
// CHECK: witness_table {{.*}} : witness_table_t(%IOps)(%S)
// CHECK: witness_table_entry(%{{[A-Za-z0-9_]+}},[[G]])
